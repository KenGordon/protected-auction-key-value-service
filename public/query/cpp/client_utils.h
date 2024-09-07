/*
 * Copyright 2023 Google LLC
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

#ifndef PUBLIC_QUERY_CPP_CLIENT_UTILS_H_
#define PUBLIC_QUERY_CPP_CLIENT_UTILS_H_

#include <string>

#include "absl/status/statusor.h"
#include "public/query/v2/get_values_v2.pb.h"

namespace kv_server {

absl::StatusOr<std::string> ToJson(const v2::GetValuesRequest& request_proto);

}  // namespace kv_server

#endif  // PUBLIC_QUERY_CPP_CLIENT_UTILS_H_

/*
 * Copyright 2023 Google LLC
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

#ifndef PUBLIC_QUERY_CPP_CLIENT_UTILS_H_
#define PUBLIC_QUERY_CPP_CLIENT_UTILS_H_

#include <string>

#include "absl/status/statusor.h"
#include "public/query/v2/get_values_v2.pb.h"

namespace kv_server {

absl::StatusOr<std::string> ToJson(const v2::GetValuesRequest& request_proto);

}  // namespace kv_server

#endif  // PUBLIC_QUERY_CPP_CLIENT_UTILS_H_
