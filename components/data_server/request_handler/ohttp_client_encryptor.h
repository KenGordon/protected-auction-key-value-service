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

#ifndef COMPONENTS_DATA_SERVER_REQUEST_HANDLER_OHTTP_CLIENT_ENCRYPTOR_H_
#define COMPONENTS_DATA_SERVER_REQUEST_HANDLER_OHTTP_CLIENT_ENCRYPTOR_H_

#include <string>
#include <string_view>

#include "absl/status/statusor.h"
#include "absl/strings/escaping.h"
#include "public/constants.h"
#include "quiche/oblivious_http/oblivious_http_client.h"

namespace kv_server {

// Handles client side encyption of requests and decryptions of responses.
// Not thread safe. Supports serial encryption/decryption per request.
class OhttpClientEncryptor {
 public:
  // Encrypts ougoing request.
  absl::StatusOr<std::string> EncryptRequest(std::string payload);
  // Decrypts incoming reponse. Since OHTTP is stateful, this method should be
  // called after EncryptRequest.
  // In order to avoid an extra copy, leaking the `ObliviousHttpResponse`.
  // Note that we have a CL for the underlying library that might allow us to
  // not do leak this object and not do the copy. If/when that's merged, we
  // should refactor this back to returning a string.
  absl::StatusOr<quiche::ObliviousHttpResponse> DecryptResponse(
      std::string encrypted_payload);

 private:
  std::optional<quiche::ObliviousHttpClient> http_client_;
  std::optional<quiche::ObliviousHttpRequest::Context> http_request_context_;

  const std::string test_public_key_ = absl::HexStringToBytes(kTestPublicKey);
  const uint8_t test_key_id = 1;
};

}  // namespace kv_server

#endif  // COMPONENTS_DATA_SERVER_REQUEST_HANDLER_OHTTP_CLIENT_ENCRYPTOR_H_
