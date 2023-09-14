/**
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

variable "service" {
  description = "Assigned name of the KV server."
  type        = string
}

variable "environment" {
  description = "Assigned environment name to group related resources."
  type        = string
}

variable "service_port" {
  description = "The grpc port that receives traffic destined for the frontend service."
  type        = number
}

variable "kv_server_address" {
  description = "gRPC-compatible address. Example: xds:///example_address"
  type        = string
}

variable "project_id" {
  description = "GCP project id."
  type        = string
}

variable "instance_groups" {
  description = "Kv server instance group URsL created by instance group managers."
  type        = set(string)
}

variable "collector_forwarding_rule" {
  description = "collector forwarding rule"
  type        = any
}

variable "collector_tcp_proxy" {
  description = "The tcp proxy for collector"
  type        = any
}
