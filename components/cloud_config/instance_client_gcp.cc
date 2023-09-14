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

// TODO(b/296901861): Modify the implementation with GCP specific logic (the
// current implementation is copied from local).

#include <memory>
#include <string>
#include <string_view>
#include <thread>

#include "absl/flags/flag.h"
#include "absl/log/check.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/synchronization/notification.h"
#include "cc/public/core/interface/execution_result.h"
#include "cc/public/cpio/interface/instance_client/instance_client_interface.h"
#include "components/cloud_config/instance_client.h"
#include "glog/logging.h"

ABSL_FLAG(std::string, shard_num, "0", "Shard number.");

namespace kv_server {
namespace {
using google::cmrt::sdk::instance_service::v1::
    GetCurrentInstanceResourceNameRequest;
using google::cmrt::sdk::instance_service::v1::
    GetCurrentInstanceResourceNameResponse;
using google::cmrt::sdk::instance_service::v1::
    GetInstanceDetailsByResourceNameRequest;
using google::cmrt::sdk::instance_service::v1::
    GetInstanceDetailsByResourceNameResponse;
using ::google::scp::core::ExecutionResult;
using ::google::scp::core::errors::GetErrorMessage;
using google::scp::cpio::InstanceClientInterface;
using google::scp::cpio::InstanceClientOptions;
using privacy_sandbox::server_common::MetricsRecorder;

constexpr std::string_view kEnvironment = "environment";

class GcpInstanceClient : public InstanceClient {
 public:
  GcpInstanceClient()
      : instance_client_(google::scp::cpio::InstanceClientFactory::Create(
            InstanceClientOptions())) {
    instance_client_->Init();
  }

  absl::StatusOr<std::string> GetEnvironmentTag() override {
    if (environment_.empty()) {
      absl::Status result = GetInstanceDetails();
      if (!result.ok()) {
        return result;
      }
    }
    if (environment_.empty()) {
      return absl::UnavailableError("Environment label not found.");
    }
    return environment_;
  }

  absl::StatusOr<std::string> GetShardNumTag() override {
    return absl::GetFlag(FLAGS_shard_num);
  }

  absl::Status RecordLifecycleHeartbeat(
      std::string_view lifecycle_hook_name) override {
    LOG(INFO) << "Record lifecycle heartbeat.";
    return absl::OkStatus();
  }

  absl::Status CompleteLifecycle(
      std::string_view lifecycle_hook_name) override {
    LOG(INFO) << "Complete lifecycle.";
    return absl::OkStatus();
  }

  absl::StatusOr<std::string> GetInstanceId() override {
    if (instance_id_.empty()) {
      absl::Status result = GetInstanceDetails();
      if (!result.ok()) {
        return result;
      }
    }
    if (instance_id_.empty()) {
      return absl::UnavailableError("Environment label not found.");
    }
    return instance_id_;
  }

  absl::StatusOr<std::vector<InstanceInfo>> DescribeInstanceGroupInstances(
      const absl::flat_hash_set<std::string>& instance_groups) override {
    auto id = GetInstanceId();
    return DescribeInstances({});
  }

  absl::StatusOr<std::vector<InstanceInfo>> DescribeInstances(
      const absl::flat_hash_set<std::string>& instance_ids) override {
    auto id = GetInstanceId();
    if (!id.ok()) {
      return id.status();
    }
    return std::vector<InstanceInfo>{InstanceInfo{.id = *id}};
  }

 private:
  std::string instance_id_;
  std::string environment_;
  std::unique_ptr<InstanceClientInterface> instance_client_;

  absl::Status GetInstanceDetails() {
    absl::StatusOr<std::string> resource_name =
        GetResourceName(instance_client_);
    if (!resource_name.ok()) {
      return resource_name.status();
    }

    absl::Notification done;
    GetInstanceDetailsByResourceNameRequest request;
    request.set_instance_resource_name(resource_name.value());

    const auto& result = instance_client_->GetInstanceDetailsByResourceName(
        std::move(request),
        [&done, this](
            const ExecutionResult& result,
            const GetInstanceDetailsByResourceNameResponse& response) {
          if (result.Successful()) {
            VLOG(2) << response.DebugString();
            instance_id_ =
                std::string{response.instance_details().instance_id()};
            environment_ =
                response.instance_details().labels().at(kEnvironment);
          } else {
            LOG(ERROR) << "Failed to get instance details: "
                       << GetErrorMessage(result.status_code);
          }
          done.Notify();
        });
    done.WaitForNotification();
    return result.Successful()
               ? absl::OkStatus()
               : absl::InternalError(GetErrorMessage(result.status_code));
  }

  absl::StatusOr<std::string> GetResourceName(
      std::unique_ptr<InstanceClientInterface>& instance_client) {
    std::string resource_name;
    absl::Notification done;
    const auto& result = instance_client->GetCurrentInstanceResourceName(
        GetCurrentInstanceResourceNameRequest(),
        [&](const ExecutionResult& result,
            const GetCurrentInstanceResourceNameResponse& response) {
          if (result.Successful()) {
            resource_name = std::string{response.instance_resource_name()};
          } else {
            LOG(ERROR) << "Faild to get instance resource name: "
                       << GetErrorMessage(result.status_code);
          }

          done.Notify();
        });
    if (!result.Successful()) {
      return absl::InternalError(GetErrorMessage(result.status_code));
    }
    done.WaitForNotification();
    if (resource_name.empty()) {
      return absl::InternalError("Failed to fetch instance resource name.");
    }
    return resource_name;
  }
};
}  // namespace

std::unique_ptr<InstanceClient> InstanceClient::Create(
    MetricsRecorder& metrics_recorder) {
  return std::make_unique<GcpInstanceClient>();
}
}  // namespace kv_server
