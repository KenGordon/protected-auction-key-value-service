/*
 * Copyright 2023 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef COMPONENTS_TELEMETRY_SERVER_DEFINITION_H_
#define COMPONENTS_TELEMETRY_SERVER_DEFINITION_H_

#include <memory>
#include <string>
#include <utility>

#include "components/telemetry/error_code.h"
#include "scp/cc/core/common/uuid/src/uuid.h"
#include "src/cpp/metric/context_map.h"
#include "src/cpp/util/read_system.h"

namespace kv_server {

constexpr std::string_view kKVServerServiceName = "KVServer";
constexpr std::string_view kInternalLookupServiceName = "InternalLookupServer";

// When server is running in debug mode, all unsafe metrics will be logged
// safely without DP noise applied. Therefore for now it is okay to set DP
// upper and lower bounds for all unsafe metrics to a default value. But
// need to revisit and tune the upper and lower bound parameters once we have
// metric monitoring set up.
// TODO(b/307362951): Tune the upper bound and lower bound for different
// unsafe metrics.
constexpr int kCounterDPLowerBound = 1;
constexpr int kCounterDPUpperBound = 10;

constexpr int kMicroSecondsLowerBound = 1;
constexpr int kMicroSecondsUpperBound = 2'000'000'000;

inline constexpr double kLatencyInMicroSecondsBoundaries[] = {
    160,     220,       280,       320,       640,       1'200,         2'500,
    5'000,   10'000,    20'000,    40'000,    80'000,    160'000,       320'000,
    640'000, 1'000'000, 1'300'000, 2'600'000, 5'000'000, 10'000'000'000};

// String literals for absl status partition, the string list and literals match
// those implemented in the absl::StatusCodeToString method
// https://github.com/abseil/abseil-cpp/blob/1a03fb9dd1c533e42b6d7d1ebea85b448a07e793/absl/status/status.cc#L47
// Strings in the partition are required to be sorted
inline constexpr absl::string_view kAbslStatusStrings[] = {
    "",
    "ABORTED",
    "ALREADY_EXISTS",
    "CANCELLED",
    "DATA_LOSS",
    "DEADLINE_EXCEEDED",
    "FAILED_PRECONDITION",
    "INTERNAL",
    "INVALID_ARGUMENT",
    "NOT_FOUND",
    "OK",
    "OUT_OF_RANGE",
    "PERMISSION_DENIED",
    "RESOURCE_EXHAUSTED",
    "UNAUTHENTICATED",
    "UNAVAILABLE",
    "UNIMPLEMENTED",
    "UNKNOWN"};

inline constexpr privacy_sandbox::server_common::metrics::PrivacyBudget
    privacy_total_budget{/*epsilon*/ 5};

// Metric definitions for request level metrics that are privacy impacting
// and should be logged unsafe with DP(differential privacy) noises.
inline constexpr privacy_sandbox::server_common::metrics::Definition<
    int, privacy_sandbox::server_common::metrics::Privacy::kImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kUpDownCounter>
    kInternalRunQueryKeySetRetrievalFailure(
        "InternalRunQueryKeySetRetrievalFailure",
        "Number of key set internal retrieval failures during internal"
        "run query processing",
        kCounterDPUpperBound, kCounterDPLowerBound);

inline constexpr privacy_sandbox::server_common::metrics::Definition<
    int, privacy_sandbox::server_common::metrics::Privacy::kImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kUpDownCounter>
    kKeysNotFoundInKeySetsInShardedLookup(
        "KeysNotFoundInKeySetsInShardedLookup",
        "Number of keys not found in the result key set in the sharded lookup",
        kCounterDPUpperBound, kCounterDPLowerBound);

inline constexpr privacy_sandbox::server_common::metrics::Definition<
    int, privacy_sandbox::server_common::metrics::Privacy::kImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kUpDownCounter>
    kKeysNotFoundInKeySetsInLocalLookup(
        "KeysNotFoundInKeySetsInLocalLookup",
        "Number of keys not found in the result key set in the local lookup",
        kCounterDPUpperBound, kCounterDPLowerBound);

inline constexpr privacy_sandbox::server_common::metrics::Definition<
    int, privacy_sandbox::server_common::metrics::Privacy::kImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kUpDownCounter>
    kInternalRunQueryEmtpyQuery("InternalRunQueryEmtpyQuery",
                                "Number of empty queries encountered during "
                                "internal run query processing",
                                kCounterDPUpperBound, kCounterDPLowerBound);

inline constexpr privacy_sandbox::server_common::metrics::Definition<
    int, privacy_sandbox::server_common::metrics::Privacy::kImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kUpDownCounter>
    kInternalRunQueryMissingKeySet(
        "InternalRunQueryMissingKeySet",
        "Number of missing keys not found in the key set "
        "during internal run query processing",
        kCounterDPUpperBound, kCounterDPLowerBound);

inline constexpr privacy_sandbox::server_common::metrics::Definition<
    int, privacy_sandbox::server_common::metrics::Privacy::kImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kUpDownCounter>
    kInternalRunQueryParsingFailure(
        "InternalRunQueryParsingFailure",
        "Number of failures in parsing query during "
        "internal run query processing",
        kCounterDPUpperBound, kCounterDPLowerBound);

inline constexpr privacy_sandbox::server_common::metrics::Definition<
    int, privacy_sandbox::server_common::metrics::Privacy::kImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kUpDownCounter>
    kLookupClientMissing(
        "LookupClientMissing",
        "Number of missing internal lookup clients encountered during "
        "sharded lookup",
        kCounterDPUpperBound, kCounterDPLowerBound);

inline constexpr privacy_sandbox::server_common::metrics::Definition<
    int, privacy_sandbox::server_common::metrics::Privacy::kImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kUpDownCounter>
    kShardedLookupServerRequestFailed(
        "ShardedLookupServerRequestFailed",
        "Number of failed server requests in the sharded lookup",
        kCounterDPUpperBound, kCounterDPLowerBound);

inline constexpr privacy_sandbox::server_common::metrics::Definition<
    int, privacy_sandbox::server_common::metrics::Privacy::kImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kUpDownCounter>
    kShardedLookupServerKeyCollisionOnCollection(
        "ShardedLookupServerKeyCollisionOnCollection",
        "Number of key collisions when collecting results from shards",
        kCounterDPUpperBound, kCounterDPLowerBound);

inline constexpr privacy_sandbox::server_common::metrics::Definition<
    int, privacy_sandbox::server_common::metrics::Privacy::kImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kUpDownCounter>
    kLookupFuturesCreationFailure(
        "LookupFuturesCreationFailure",
        "Number of failures in creating lookup futures in the sharded lookup",
        kCounterDPUpperBound, kCounterDPLowerBound);

inline constexpr privacy_sandbox::server_common::metrics::Definition<
    int, privacy_sandbox::server_common::metrics::Privacy::kImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kUpDownCounter>
    kShardedLookupFailure("ShardedLookupFailure",
                          "Number of lookup failures in the sharded lookup",
                          kCounterDPUpperBound, kCounterDPLowerBound);

inline constexpr privacy_sandbox::server_common::metrics::Definition<
    int, privacy_sandbox::server_common::metrics::Privacy::kImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kUpDownCounter>
    kRemoteClientEncryptionFailure(
        "RemoteClientEncryptionFailure",
        "Number of request encryption failures in the remote lookup client",
        kCounterDPUpperBound, kCounterDPLowerBound);

inline constexpr privacy_sandbox::server_common::metrics::Definition<
    int, privacy_sandbox::server_common::metrics::Privacy::kImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kUpDownCounter>
    kRemoteClientSecureLookupFailure(
        "RemoteClientSecureLookupFailure",
        "Number of secure lookup failures in the remote lookup client",
        kCounterDPUpperBound, kCounterDPLowerBound);

inline constexpr privacy_sandbox::server_common::metrics::Definition<
    int, privacy_sandbox::server_common::metrics::Privacy::kImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kUpDownCounter>
    kRemoteClientDecryptionFailure(
        "RemoteClientDecryptionFailure",
        "Number of response decryption failures in the remote lookup client",
        kCounterDPUpperBound, kCounterDPLowerBound);

inline constexpr privacy_sandbox::server_common::metrics::Definition<
    int, privacy_sandbox::server_common::metrics::Privacy::kImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kUpDownCounter>
    kInternalClientDecryptionFailure(
        "InternalClientEncryptionFailure",
        "Number of request decryption failures in the internal lookup client",
        kCounterDPUpperBound, kCounterDPLowerBound);

inline constexpr privacy_sandbox::server_common::metrics::Definition<
    int, privacy_sandbox::server_common::metrics::Privacy::kImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kUpDownCounter>
    kInternalClientUnpaddingRequestError(
        "InternalClientUnpaddingRequestError",
        "Number of unpadding errors in the request deserialization in the "
        "internal lookup client",
        kCounterDPUpperBound, kCounterDPLowerBound);

inline constexpr privacy_sandbox::server_common::metrics::Definition<
    int, privacy_sandbox::server_common::metrics::Privacy::kImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kHistogram>
    kShardedLookupRunQueryLatencyInMicros(
        "ShardedLookupRunQueryLatencyInMicros",
        "Latency in executing run query in the sharded lookup",
        kLatencyInMicroSecondsBoundaries, kMicroSecondsUpperBound,
        kMicroSecondsLowerBound);

inline constexpr privacy_sandbox::server_common::metrics::Definition<
    int, privacy_sandbox::server_common::metrics::Privacy::kImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kHistogram>
    kRemoteLookupGetValuesLatencyInMicros(
        "RemoteLookupGetValuesLatencyInMicros",
        "Latency in get values in the remote lookup",
        kLatencyInMicroSecondsBoundaries, kMicroSecondsUpperBound,
        kMicroSecondsLowerBound);

inline constexpr privacy_sandbox::server_common::metrics::Definition<
    int, privacy_sandbox::server_common::metrics::Privacy::kImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kHistogram>
    kInternalSecureLookupLatencyInMicros("InternalSecureLookupLatencyInMicros",
                                         "Latency in internal secure lookup",
                                         kLatencyInMicroSecondsBoundaries,
                                         kMicroSecondsUpperBound,
                                         kMicroSecondsLowerBound);

inline constexpr privacy_sandbox::server_common::metrics::Definition<
    int, privacy_sandbox::server_common::metrics::Privacy::kImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kHistogram>
    kGetValuePairsLatencyInMicros("GetValuePairsLatencyInMicros",
                                  "Latency in executing GetValuePairs in cache",
                                  kLatencyInMicroSecondsBoundaries,
                                  kMicroSecondsUpperBound,
                                  kMicroSecondsLowerBound);

inline constexpr privacy_sandbox::server_common::metrics::Definition<
    int, privacy_sandbox::server_common::metrics::Privacy::kImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kHistogram>
    kGetKeyValueSetLatencyInMicros(
        "GetKeyValueSetLatencyInMicros",
        "Latency in executing GetKeyValueSet in cache",
        kLatencyInMicroSecondsBoundaries, kMicroSecondsUpperBound,
        kMicroSecondsLowerBound);

// Metric definitions for safe metrics that are not privacy impacting
inline constexpr privacy_sandbox::server_common::metrics::Definition<
    int, privacy_sandbox::server_common::metrics::Privacy::kNonImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kPartitionedCounter>
    kGetParameterStatus("GetParameterStatus", "Get parameter status", "status",
                        kAbslStatusStrings);

inline constexpr privacy_sandbox::server_common::metrics::Definition<
    int, privacy_sandbox::server_common::metrics::Privacy::kNonImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kPartitionedCounter>
    kCompleteLifecycleStatus("CompleteLifecycleStatus",
                             "Server complete life cycle status", "status",
                             kAbslStatusStrings);

inline constexpr privacy_sandbox::server_common::metrics::Definition<
    int, privacy_sandbox::server_common::metrics::Privacy::kNonImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kPartitionedCounter>
    kCreateDataOrchestratorStatus("CreateDataOrchestratorStatus",
                                  "Data orchestrator creation status", "status",
                                  kAbslStatusStrings);

inline constexpr privacy_sandbox::server_common::metrics::Definition<
    int, privacy_sandbox::server_common::metrics::Privacy::kNonImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kPartitionedCounter>
    kStartDataOrchestratorStatus("StartDataOrchestratorStatus",
                                 "Data orchestrator start status count",
                                 "status", kAbslStatusStrings);

inline constexpr privacy_sandbox::server_common::metrics::Definition<
    int, privacy_sandbox::server_common::metrics::Privacy::kNonImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kPartitionedCounter>
    kLoadNewFilesStatus("LoadNewFilesStatus", "Load new file status", "status",
                        kAbslStatusStrings);

inline constexpr privacy_sandbox::server_common::metrics::Definition<
    int, privacy_sandbox::server_common::metrics::Privacy::kNonImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kPartitionedCounter>
    kGetShardManagerStatus("GetShardManagerStatus", "Get shard manager status",
                           "status", kAbslStatusStrings);

inline constexpr privacy_sandbox::server_common::metrics::Definition<
    int, privacy_sandbox::server_common::metrics::Privacy::kNonImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kPartitionedCounter>
    kDescribeInstanceGroupInstancesStatus(
        "DescribeInstanceGroupInstancesStatus",
        "Describe instance group instances status", "status",
        kAbslStatusStrings);

inline constexpr privacy_sandbox::server_common::metrics::Definition<
    int, privacy_sandbox::server_common::metrics::Privacy::kNonImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kPartitionedCounter>
    kDescribeInstancesStatus("DescribeInstancesStatus",
                             "Describe instances status", "status",
                             kAbslStatusStrings);

inline constexpr privacy_sandbox::server_common::metrics::Definition<
    double, privacy_sandbox::server_common::metrics::Privacy::kNonImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kUpDownCounter>
    kRealtimeTotalRowsUpdated("RealtimeTotalRowsUpdated",
                              "Realtime total rows updated count");

inline constexpr privacy_sandbox::server_common::metrics::Definition<
    double, privacy_sandbox::server_common::metrics::Privacy::kNonImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kHistogram>
    kReceivedLowLatencyNotificationsE2ECloudProvided(
        "ReceivedLowLatencyNotificationsE2ECloudProvided",
        "Time between cloud topic publisher inserting message and realtime "
        "notifier receiving the message",
        kLatencyInMicroSecondsBoundaries);

inline constexpr privacy_sandbox::server_common::metrics::Definition<
    double, privacy_sandbox::server_common::metrics::Privacy::kNonImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kHistogram>
    kReceivedLowLatencyNotificationsE2E(
        "ReceivedLowLatencyNotificationsE2E",
        "Time between producer producing the message and realtime notifier "
        "receiving the message",
        kLatencyInMicroSecondsBoundaries);

inline constexpr privacy_sandbox::server_common::metrics::Definition<
    double, privacy_sandbox::server_common::metrics::Privacy::kNonImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kHistogram>
    kReceivedLowLatencyNotifications(
        "ReceivedLowLatencyNotifications",
        "Latency in realtime notifier processing the received batch of "
        "notification messages",
        kLatencyInMicroSecondsBoundaries);

inline constexpr privacy_sandbox::server_common::metrics::Definition<
    double, privacy_sandbox::server_common::metrics::Privacy::kNonImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kHistogram>
    kAwsSqsReceiveMessageLatency("AwsSqsReceiveMessageLatency",
                                 "AWS SQS receive message latency",
                                 kLatencyInMicroSecondsBoundaries);

inline constexpr privacy_sandbox::server_common::metrics::Definition<
    int, privacy_sandbox::server_common::metrics::Privacy::kNonImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kPartitionedCounter>
    kChangeNotifierErrors("ChangeNotifierErrors",
                          "Errors in the change notifier", "error_code",
                          kChangeNotifierErrorCode);

inline constexpr privacy_sandbox::server_common::metrics::Definition<
    int, privacy_sandbox::server_common::metrics::Privacy::kNonImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kPartitionedCounter>
    kRealtimeErrors("RealtimeErrors", "Errors in realtime data loading",
                    "error_code", kRealtimeErrorCode);

inline constexpr privacy_sandbox::server_common::metrics::Definition<
    double, privacy_sandbox::server_common::metrics::Privacy::kNonImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kHistogram>
    kSeekingInputStreambufSizeLatency("SeekingInputStreambufSizeLatency",
                                      "Latency in seeking input streambuf size",
                                      kLatencyInMicroSecondsBoundaries);

inline constexpr privacy_sandbox::server_common::metrics::Definition<
    double, privacy_sandbox::server_common::metrics::Privacy::kNonImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kHistogram>
    kSeekingInputStreambufUnderflowLatency(
        "SeekingInputStreambufUnderflowLatency",
        "Latency in seeking input streambuf underflow",
        kLatencyInMicroSecondsBoundaries);

inline constexpr privacy_sandbox::server_common::metrics::Definition<
    double, privacy_sandbox::server_common::metrics::Privacy::kNonImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kHistogram>
    kSeekingInputStreambufSeekoffLatency(
        "SeekingInputStreambufSeekoffLatency",
        "Latency in seeking input streambuf seekoff",
        kLatencyInMicroSecondsBoundaries);

inline constexpr privacy_sandbox::server_common::metrics::Definition<
    double, privacy_sandbox::server_common::metrics::Privacy::kNonImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kUpDownCounter>
    kTotalRowsDroppedInDataLoading("TotalRowsDroppedInDataLoading",
                                   "Total rows dropped during data loading");

inline constexpr privacy_sandbox::server_common::metrics::Definition<
    double, privacy_sandbox::server_common::metrics::Privacy::kNonImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kUpDownCounter>
    kTotalRowsUpdatedInDataLoading("TotalRowsUpdatedInDataLoading",
                                   "Total rows updated during data loading");

inline constexpr privacy_sandbox::server_common::metrics::Definition<
    double, privacy_sandbox::server_common::metrics::Privacy::kNonImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kUpDownCounter>
    kTotalRowsDeletedInDataLoading("TotalRowsDeletedInDataLoading",
                                   "Total rows deleted during data loading");

inline constexpr privacy_sandbox::server_common::metrics::Definition<
    double, privacy_sandbox::server_common::metrics::Privacy::kNonImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kHistogram>
    kConcurrentStreamRecordReaderReadShardRecordsLatency(
        "ConcurrentStreamRecordReaderReadShardRecordsLatency",
        "Latency in ConcurrentStreamRecordReader reading shard records",
        kLatencyInMicroSecondsBoundaries);

inline constexpr privacy_sandbox::server_common::metrics::Definition<
    double, privacy_sandbox::server_common::metrics::Privacy::kNonImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kHistogram>
    kConcurrentStreamRecordReaderReadStreamRecordsLatency(
        "ConcurrentStreamRecordReaderReadStreamRecordsLatency",
        "Latency in ConcurrentStreamRecordReader reading stream records",
        kLatencyInMicroSecondsBoundaries);

inline constexpr privacy_sandbox::server_common::metrics::Definition<
    double, privacy_sandbox::server_common::metrics::Privacy::kNonImpacting,
    privacy_sandbox::server_common::metrics::Instrument::kHistogram>
    kConcurrentStreamRecordReaderReadByteRangeLatency(
        "ConcurrentStreamRecordReaderReadByteRangeLatency",
        "Latency in ConcurrentStreamRecordReader reading byte range",
        kLatencyInMicroSecondsBoundaries);

// KV server metrics list contains contains non request related safe metrics
// and request metrics collected before stage of internal lookups
inline constexpr const privacy_sandbox::server_common::metrics::DefinitionName*
    kKVServerMetricList[] = {
        // Unsafe metrics
        &kKeysNotFoundInKeySetsInShardedLookup,
        &kShardedLookupServerRequestFailed,
        &kShardedLookupServerKeyCollisionOnCollection, &kLookupClientMissing,
        &kLookupFuturesCreationFailure, &kShardedLookupFailure,
        &kRemoteClientEncryptionFailure, &kRemoteClientSecureLookupFailure,
        &kRemoteClientDecryptionFailure, &kShardedLookupRunQueryLatencyInMicros,
        &kRemoteLookupGetValuesLatencyInMicros,
        // Safe metrics
        &privacy_sandbox::server_common::metrics::kServerTotalTimeMs,
        &kGetParameterStatus, &kCompleteLifecycleStatus,
        &kCreateDataOrchestratorStatus, &kStartDataOrchestratorStatus,
        &kLoadNewFilesStatus, &kGetShardManagerStatus,
        &kDescribeInstanceGroupInstancesStatus, &kDescribeInstancesStatus,
        &kRealtimeTotalRowsUpdated,
        &kReceivedLowLatencyNotificationsE2ECloudProvided,
        &kReceivedLowLatencyNotificationsE2E, &kReceivedLowLatencyNotifications,
        &kChangeNotifierErrors, &kRealtimeErrors, &kAwsSqsReceiveMessageLatency,
        &kSeekingInputStreambufSeekoffLatency,
        &kSeekingInputStreambufSizeLatency,
        &kSeekingInputStreambufUnderflowLatency,
        &kTotalRowsDroppedInDataLoading, &kTotalRowsUpdatedInDataLoading,
        &kTotalRowsDeletedInDataLoading,
        &kConcurrentStreamRecordReaderReadShardRecordsLatency,
        &kConcurrentStreamRecordReaderReadStreamRecordsLatency,
        &kConcurrentStreamRecordReaderReadByteRangeLatency};

// Internal lookup service metrics list contains metrics collected in the
// internal lookup server. This separation from KV metrics list allows all
// lookup requests (local and request from remote KV server) to contribute to
// the same set of metrics, so that the noise of unsafe metric won't be skewed
// for particular batch of requests, e.g server request that requires only
// remote lookups
inline constexpr const privacy_sandbox::server_common::metrics::DefinitionName*
    kInternalLookupServiceMetricsList[] = {
        // Unsafe metrics
        &kInternalRunQueryKeySetRetrievalFailure,
        &kKeysNotFoundInKeySetsInLocalLookup,
        &kInternalRunQueryEmtpyQuery,
        &kInternalRunQueryMissingKeySet,
        &kInternalRunQueryParsingFailure,
        &kInternalClientDecryptionFailure,
        &kInternalClientUnpaddingRequestError,
        &kInternalSecureLookupLatencyInMicros,
        &kGetValuePairsLatencyInMicros,
        &kGetKeyValueSetLatencyInMicros};

inline constexpr absl::Span<
    const privacy_sandbox::server_common::metrics::DefinitionName* const>
    kKVServerMetricSpan = kKVServerMetricList;

inline constexpr absl::Span<
    const privacy_sandbox::server_common::metrics::DefinitionName* const>
    kInternalLookupServiceMetricsSpan = kInternalLookupServiceMetricsList;

inline auto* KVServerContextMap(
    std::optional<
        privacy_sandbox::server_common::telemetry::BuildDependentConfig>
        config = std::nullopt,
    std::unique_ptr<opentelemetry::metrics::MeterProvider> provider = nullptr,
    absl::string_view service = kKVServerServiceName,
    absl::string_view version = "") {
  return privacy_sandbox::server_common::metrics::GetContextMap<
      const std::string, kKVServerMetricSpan>(std::move(config),
                                              std::move(provider), service,
                                              version, privacy_total_budget);
}

inline auto* InternalLookupServerContextMap(
    std::optional<
        privacy_sandbox::server_common::telemetry::BuildDependentConfig>
        config = std::nullopt,
    std::unique_ptr<opentelemetry::metrics::MeterProvider> provider = nullptr,
    absl::string_view service = kInternalLookupServiceName,
    absl::string_view version = "") {
  return privacy_sandbox::server_common::metrics::GetContextMap<
      const std::string, kInternalLookupServiceMetricsSpan>(
      std::move(config), std::move(provider), service, version,
      privacy_total_budget);
}

template <typename T>
inline void AddSystemMetric(T* context_map) {
  context_map->AddObserverable(
      privacy_sandbox::server_common::metrics::kCpuPercent,
      privacy_sandbox::server_common::GetCpu);
  context_map->AddObserverable(
      privacy_sandbox::server_common::metrics::kMemoryKB,
      privacy_sandbox::server_common::GetMemory);
}

inline void LogIfError(const absl::Status& s,
                       absl::string_view message = "when logging metric",
                       privacy_sandbox::server_common::SourceLocation location
                           PS_LOC_CURRENT_DEFAULT_ARG) {
  if (s.ok()) return;
  ABSL_LOG_EVERY_N_SEC(WARNING, 60)
          .AtLocation(location.file_name(), location.line())
      << message << ": " << s;
}

template <const auto& definition>
inline absl::AnyInvocable<void(const absl::Status&, int) const>
LogStatusSafeMetricsFn() {
  return [](const absl::Status& status, int count) {
    LogIfError(KVServerContextMap()->SafeMetric().LogUpDownCounter<definition>(
        {{absl::StatusCodeToString(status.code()), count}}));
  };
}

inline absl::AnyInvocable<void(const absl::Status&, int) const>
LogMetricsNoOpCallback() {
  return [](const absl::Status& status, int count) {};
}

// Initialize metrics context map
// This is the minimum requirement to initialize the noop telemetry
inline void InitMetricsContextMap() {
  privacy_sandbox::server_common::telemetry::TelemetryConfig config_proto;
  config_proto.set_mode(
      privacy_sandbox::server_common::telemetry::TelemetryConfig::PROD);
  kv_server::KVServerContextMap(
      privacy_sandbox::server_common::telemetry::BuildDependentConfig(
          config_proto));
  kv_server::InternalLookupServerContextMap(
      privacy_sandbox::server_common::telemetry::BuildDependentConfig(
          config_proto));
}

using UdfRequestMetricsContext =
    privacy_sandbox::server_common::metrics::ServerContext<kKVServerMetricSpan>;
using InternalLookupMetricsContext =
    privacy_sandbox::server_common::metrics::ServerContext<
        kInternalLookupServiceMetricsSpan>;

// ScopeMetricsContext provides metrics context ties to the request and
// should have the same lifetime of the request.
// The purpose of this class is to avoid explicit creating and deleting metrics
// context from context map. The metrics context associated with the request
// will be destroyed after ScopeMetricsContext goes out of scope.
class ScopeMetricsContext {
 public:
  explicit ScopeMetricsContext(
      std::string request_id = google::scp::core::common::ToString(
          google::scp::core::common::Uuid::GenerateUuid()))
      : request_id_(std::move(request_id)) {
    // Create a metrics context in the context map and
    // associated it with request id
    KVServerContextMap()->Get(&request_id_);
    CHECK_OK([this]() {
      // Remove the metrics context for request_id to transfer the ownership
      // of metrics context to the ScopeMetricsContext. This is to ensure that
      // metrics context has the same lifetime with RequestContext and be
      // destroyed when ScopeMetricsContext goes out of scope.
      PS_ASSIGN_OR_RETURN(udf_request_metrics_context_,
                          KVServerContextMap()->Remove(&request_id_));
      return absl::OkStatus();
    }()) << "Udf request metrics context is not initialized";
    InternalLookupServerContextMap()->Get(&request_id_);
    CHECK_OK([this]() {
      // Remove the metrics context for request_id to transfer the ownership
      // of metrics context to the ScopeMetricsContext. This is to ensure that
      // metrics context has the same lifetime with RequestContext and be
      // destroyed when ScopeMetricsContext goes out of scope.
      PS_ASSIGN_OR_RETURN(
          internal_lookup_metrics_context_,
          InternalLookupServerContextMap()->Remove(&request_id_));
      return absl::OkStatus();
    }()) << "Internal lookup metrics context is not initialized";
  }
  UdfRequestMetricsContext& GetUdfRequestMetricsContext() const {
    return *udf_request_metrics_context_;
  }
  InternalLookupMetricsContext& GetInternalLookupMetricsContext() const {
    return *internal_lookup_metrics_context_;
  }

 private:
  const std::string request_id_;
  // Metrics context has the same lifetime of server request context
  std::unique_ptr<UdfRequestMetricsContext> udf_request_metrics_context_;
  std::unique_ptr<InternalLookupMetricsContext>
      internal_lookup_metrics_context_;
};

}  // namespace kv_server

#endif  // COMPONENTS_TELEMETRY_SERVER_DEFINITION_H_
