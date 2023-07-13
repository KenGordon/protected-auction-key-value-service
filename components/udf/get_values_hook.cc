// Copyright 2022 Google LLC
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

#include "components/udf/get_values_hook.h"

#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "absl/functional/any_invocable.h"
#include "absl/status/statusor.h"
#include "components/internal_server/lookup.grpc.pb.h"
#include "glog/logging.h"
#include "google/protobuf/util/json_util.h"
#include "nlohmann/json.hpp"
#include "src/cpp/telemetry/telemetry.h"

namespace kv_server {
namespace {

using google::protobuf::util::MessageToJsonString;
using privacy_sandbox::server_common::GetTracer;

class GetValuesHookImpl : public GetValuesHook {
 public:
  explicit GetValuesHookImpl(absl::AnyInvocable<std::unique_ptr<LookupClient>()>
                                 lookup_client_supplier)
      : lookup_client_supplier_(std::move(lookup_client_supplier)) {}

  std::string operator()(std::tuple<std::vector<std::string>>& input) {
    // Lazy load lookup client on first call.
    if (lookup_client_ == nullptr) {
      lookup_client_ = lookup_client_supplier_();
    }
    // TODO(b/261181061): Determine where to InitTracer.
    VLOG(9) << "Calling internal lookup client";
    absl::StatusOr<InternalLookupResponse> response_or_status =
        lookup_client_->GetValues(std::get<0>(input));

    VLOG(9) << "Processing internal lookup response";
    if (!response_or_status.ok()) {
      nlohmann::json status;
      status["code"] = response_or_status.status().code();
      status["message"] = response_or_status.status().message();
      return status.dump();
    }

    std::string kv_pairs_json;
    MessageToJsonString(response_or_status.value(), &kv_pairs_json);

    return kv_pairs_json;
  }

 private:
  // `lookup_client_` is lazy loaded because getting one can cause thread
  // creation. Lazy load is used to ensure that it only happens after Roma
  // forks.
  absl::AnyInvocable<std::unique_ptr<LookupClient>()> lookup_client_supplier_;
  std::unique_ptr<LookupClient> lookup_client_;
};
}  // namespace

std::unique_ptr<GetValuesHook> GetValuesHook::Create(
    absl::AnyInvocable<std::unique_ptr<LookupClient>()>
        lookup_client_supplier) {
  return std::make_unique<GetValuesHookImpl>(std::move(lookup_client_supplier));
}

}  // namespace kv_server
