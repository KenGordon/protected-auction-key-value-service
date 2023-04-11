// Copyright 2023 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <fstream>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/substitute.h"
#include "components/data_server/cache/cache.h"
#include "components/data_server/cache/key_value_cache.h"
#include "components/udf/cache_get_values_hook.h"
#include "components/udf/udf_client.h"
#include "glog/logging.h"
#include "nlohmann/json.hpp"
#include "public/data_loading/data_loading_generated.h"
#include "public/data_loading/readers/delta_record_stream_reader.h"
#include "public/udf/constants.h"

ABSL_FLAG(std::string, kv_delta_file_path, "",
          "Path to delta file with KV pairs.");
ABSL_FLAG(std::string, udf_delta_file_path, "", "Path to UDF delta file.");
ABSL_FLAG(std::string, key, "", "Key to send in request to UDF.");
ABSL_FLAG(std::string, subkey, "", "Context subkey to send in request to UDF.");
ABSL_FLAG(std::string, namespace_tag, "keys",
          "Namespace tag for keys (see "
          "https://github.com/WICG/turtledove/blob/main/"
          "FLEDGE_Key_Value_Server_API.md#available-tags). Defaults to `keys`. "
          "Options: `keys`, `renderUrls`, `adComponentRenderUrls`. ");

namespace kv_server {

absl::Status LoadCacheFromFile(std::string file_path, Cache& cache) {
  std::ifstream delta_file(file_path);
  DeltaRecordStreamReader record_reader(delta_file);
  absl::Status status =
      record_reader.ReadRecords([&cache](const DeltaFileRecordStruct& record) {
        switch (record.mutation_type) {
          case DeltaMutationType::Update: {
            LOG(INFO) << "Updating cache with key " << record.key << ", value "
                      << record.value << ", logical commit time "
                      << record.logical_commit_time;
            cache.UpdateKeyValue(record.key, record.value,
                                 record.logical_commit_time);
            break;
          }
          case DeltaMutationType::Delete: {
            cache.DeleteKey(record.key, record.logical_commit_time);
            break;
          }
          default:
            return absl::InvalidArgumentError(
                absl::StrCat("Invalid mutation type: ",
                             EnumNameDeltaMutationType(record.mutation_type)));
        }
        return absl::OkStatus();
      });
  return status;
}

absl::Status ReadCodeConfigFromFile(std::string file_path,
                                    CodeConfig& code_config) {
  std::ifstream delta_file(file_path);
  DeltaRecordStreamReader record_reader(delta_file);
  absl::Status status = record_reader.ReadRecords(
      [&code_config](const DeltaFileRecordStruct& record) {
        if (record.mutation_type != DeltaMutationType::Update) {
          // Ignore non-updates
          return absl::OkStatus();
        }
        if (record.key == kUdfHandlerNameKey) {
          code_config.udf_handler_name = record.value;
        }
        if (record.key == kUdfCodeSnippetKey) {
          code_config.js = record.value;
        }
        return absl::OkStatus();
      });
  if (!status.ok()) {
    return status;
  }

  if (code_config.udf_handler_name.empty()) {
    return absl::InvalidArgumentError(
        "Missing `udf_handler_name` key in delta file.");
  }
  if (code_config.js.empty()) {
    return absl::InvalidArgumentError(
        "Missing `udf_code_snippet` key in delta file.");
  }
  return absl::OkStatus();
}

absl::StatusOr<std::vector<std::string>> GetUdfInput(std::string subkey,
                                                     std::string namespace_tag,
                                                     std::string key) {
  const std::string input_string = absl::Substitute(R"({
    "context": {"subkey": "$0"},
    "keyGroups": [
      {
        "tags": [
          "custom",
          "$1"
        ],
        "keyList": [
          "$2"
        ]
      }
    ],
    "udfApiInputVersion": 1
  })",
                                                    subkey, namespace_tag, key);
  const nlohmann::json udf_input = nlohmann::json::parse(input_string);
  if (udf_input.is_discarded()) {
    return absl::InvalidArgumentError("Failed to parse the request json");
  }
  return std::vector<std::string>{udf_input.dump()};
}

absl::Status ParseUdfResult(std::string result) {
  nlohmann::json result_json = nlohmann::json::parse(result);
  if (result_json.is_discarded()) {
    return absl::InvalidArgumentError("Invalid JSON format of UDF output.");
  }
  return absl::OkStatus();
}

void ShutdownUdf(UdfClient& udf_client) {
  auto udf_client_stop = udf_client.Stop();
  if (!udf_client_stop.ok()) {
    LOG(ERROR) << "Error shutting down UDF execution engine: "
               << udf_client_stop;
  }
}

absl::Status TestUdf(std::string kv_delta_file_path,
                     std::string udf_delta_file_path, std::string key,
                     std::string subkey, std::string namespace_tag) {
  LOG(INFO) << "Loading cache from delta file: " << kv_delta_file_path;
  std::unique_ptr<Cache> cache = KeyValueCache::Create();
  auto load_cache_status = LoadCacheFromFile(kv_delta_file_path, *cache);
  if (!load_cache_status.ok()) {
    LOG(ERROR) << "Error loading cache from file: " << load_cache_status;
    return load_cache_status;
  }

  LOG(INFO) << "Starting UDF client";
  auto udf_client = UdfClient::Create(
      UdfClient::ConfigWithGetValuesHook(*NewCacheGetValuesHook(*cache), 1));
  if (!udf_client.ok()) {
    LOG(ERROR) << "Error starting UDF execution engine: "
               << udf_client.status();
    return udf_client.status();
  }

  LOG(INFO) << "Loading udf code config from delta file: "
            << udf_delta_file_path;
  CodeConfig code_config;
  auto code_config_status =
      ReadCodeConfigFromFile(udf_delta_file_path, code_config);
  if (!code_config_status.ok()) {
    LOG(ERROR) << "Error loading UDF code from file: " << code_config_status;
    ShutdownUdf(*udf_client.value());
    return code_config_status;
  }

  auto code_object_status = udf_client.value()->SetCodeObject(code_config);
  if (!code_object_status.ok()) {
    LOG(ERROR) << "Error setting UDF code object: " << code_object_status;
    ShutdownUdf(*udf_client.value());
    return code_object_status;
  }

  LOG(INFO) << "Building udf input with key: " << key;
  absl::StatusOr<std::vector<std::string>> udf_input =
      GetUdfInput(subkey, namespace_tag, key);
  if (!udf_input.ok()) {
    LOG(ERROR) << "Error building UDF input: " << udf_input.status();
    return udf_input.status();
  }

  LOG(INFO) << "Executing UDF with input:" << udf_input.value()[0];
  auto udf_result = udf_client.value()->ExecuteCode(udf_input.value());
  if (!udf_result.ok()) {
    LOG(ERROR) << "UDF execution failed: " << udf_result.status();
    ShutdownUdf(*udf_client.value());
    return udf_result.status();
  }
  ShutdownUdf(*udf_client.value());

  LOG(INFO) << "UDF unparsed result: " << udf_result.value();

  auto parse_status = ParseUdfResult(udf_result.value());
  if (!parse_status.ok()) {
    return parse_status;
  }

  std::cout << "UDF execution result: " << udf_result.value() << std::endl;

  return absl::OkStatus();
}

}  // namespace kv_server

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);

  const std::string kv_delta_file_path =
      absl::GetFlag(FLAGS_kv_delta_file_path);
  const std::string udf_delta_file_path =
      absl::GetFlag(FLAGS_udf_delta_file_path);
  const std::string key = absl::GetFlag(FLAGS_key);
  const std::string subkey = absl::GetFlag(FLAGS_subkey);
  const std::string namespace_tag = absl::GetFlag(FLAGS_namespace_tag);

  auto status = kv_server::TestUdf(kv_delta_file_path, udf_delta_file_path, key,
                                   subkey, namespace_tag);
  if (!status.ok()) {
    return -1;
  }
  return 0;
}
