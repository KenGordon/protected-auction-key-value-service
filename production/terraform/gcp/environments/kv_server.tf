# Copyright 2023 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

provider "google" {
  project = var.project_id
}

provider "google-beta" {
  project = var.project_id
}

module "kv_server" {
  source                                = "../modules/kv_server"
  project_id                            = var.project_id
  environment                           = var.environment
  service_account_email                 = var.service_account_email
  regions                               = var.regions
  gcp_image_repo                        = var.gcp_image_repo
  kv_service_port                       = var.kv_service_port
  min_replicas_per_service_region       = var.min_replicas_per_service_region
  max_replicas_per_service_region       = var.max_replicas_per_service_region
  use_confidential_space_debug_image    = var.use_confidential_space_debug_image
  vm_startup_delay_seconds              = var.vm_startup_delay_seconds
  machine_type                          = var.machine_type
  instance_template_waits_for_instances = var.instance_template_waits_for_instances
  cpu_utilization_percent               = var.cpu_utilization_percent
  collector_service_name                = var.collector_service_name
  collector_machine_type                = var.collector_machine_type
  collector_service_port                = var.collector_service_port
  collector_domain_name                 = var.collector_domain_name
  dns_zone                              = var.dns_zone
  # TODO(b/299623229): remove the following reminder once b/299623229 is done.
  # Reminder: for any new parameters added here, please also add them to "components/cloud_config/parameter_client_local.cc".
  parameters = {
    directory                               = var.directory
    data-bucket-id                          = var.data_bucket_id
    launch-hook                             = var.launch_hook
    realtime-directory                      = var.realtime_directory
    use-external-metrics-collector-endpoint = var.use_external_metrics_collector_endpoint
    metrics-collector-endpoint              = "${var.environment}-${var.collector_service_name}.${var.collector_domain_name}:${var.collector_service_port}"
    metrics-export-interval-millis          = var.metrics_export_interval_millis
    metrics-export-timeout-millis           = var.metrics_export_timeout_millis
    backup-poll-frequency-secs              = var.backup_poll_frequency_secs
    realtime-updater-num-threads            = var.realtime_updater_num_threads
    data-loading-num-threads                = var.data_loading_num_threads
    s3client-max-connections                = var.s3client_max_connections
    s3client-max-range-bytes                = var.s3client_max_range_bytes
    num-shards                              = var.num_shards
    udf-num-workers                         = var.udf_num_workers
    route-v1-to-v2                          = var.route_v1_to_v2
    use-real-coordinators                   = var.use_real_coordinators
    environment                             = var.environment
    project_id                              = var.project_id
  }

}
