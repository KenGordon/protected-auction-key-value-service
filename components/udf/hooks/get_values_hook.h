/*
 * Copyright 2022 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef COMPONENTS_UDF_GET_VALUES_HOOK_H_
#define COMPONENTS_UDF_GET_VALUES_HOOK_H_

#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "absl/functional/any_invocable.h"
#include "components/internal_server/lookup_client.h"
#include "roma/interface/function_binding_io.pb.h"

namespace kv_server {

// Functor that acts as a wrapper for the internal lookup client call.
class GetValuesHook {
 public:
  enum class OutputType { kString = 0, kBinary };

  virtual ~GetValuesHook() = default;

  // This is registered with v8 and is exposed to the UDF. Internally, it calls
  // the internal lookup client.
  virtual void operator()(
      google::scp::roma::proto::FunctionBindingIoProto& io) = 0;

  static std::unique_ptr<GetValuesHook> Create(
      absl::AnyInvocable<std::unique_ptr<LookupClient>()>
          lookup_client_supplier,
      OutputType output_type);
};

}  // namespace kv_server

#endif  // COMPONENTS_UDF_GET_VALUES_HOOK_H_
