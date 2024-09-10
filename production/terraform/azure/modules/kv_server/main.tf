# Portions Copyright (c) Microsoft Corporation
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

################ Common Setup ################

module "resource_group" {
  source                = "../../services/resource_group"
  operator              = var.operator
  environment           = var.environment
  region                = var.region
}

module "networking" {
  source                = "../../services/networking"
  resource_group_name   = module.resource_group.name
  operator              = var.operator
  environment           = var.environment
  region                = var.region
  # vnet_address_space    = var.vnet_address_space
  # default_subnet_cidr   = var.default_subnet_cidr
  # aks_subnet_cidr       = var.aks_subnet_cidr
  # cg_subnet_cidr        = var.cg_subnet_cidr
}

module "aks" {
  source                = "../../services/aks"
  resource_group_id     = module.resource_group.id
  resource_group_name   = module.resource_group.name  
  operator              = var.operator
  environment           = var.environment
  region                = var.region
  subnet_id             = module.networking.aks_subnet_id
  virtual_network_id    = module.networking.vnet_id
  # kubernetes_version    = var.kubernetes_version
  # service_cidr          = var.service_cidr
  # dns_service_ip        = var.dns_service_ip
}

module "external_dns" {
  source                     = "../../services/external_dns"
  resource_group_id          = module.resource_group.id
  resource_group_name        = module.resource_group.name
  region                     = var.region
  vnet_id                    = module.networking.vnet_id
  vnet_name                  = module.networking.vnet_name
  private_domain_name        = var.private_domain_name
  aks_cluster_name           = module.aks.name
  aks_oidc_issuer_url        = module.aks.oidc_issuer_url
  kubernetes_namespace       = var.externaldns_kubernetes_namespace
  kubernetes_service_account = var.externaldns_kubernetes_service_account
  tenant_id                  = var.tenant_id
  subscription_id            = var.subscription_id
}

module "file_storage" {
  source                = "../../services/file_storage"
  resource_group_id     = module.resource_group.id
  resource_group_name   = module.resource_group.name
  region                = var.region
}

