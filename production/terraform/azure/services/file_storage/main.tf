# generate a random string (consisting of four characters)
# https://registry.terraform.io/providers/hashicorp/random/latest/docs/resources/string
resource "random_string" "random" {
  length  = 4
  upper   = false
  special = false
}

## Azure Storage Accounts requires a globally unique names
## https://docs.microsoft.com/en-us/azure/storage/common/storage-account-overview
## Create a File Storage Account 
resource "azurerm_storage_account" "storage" {
  name                     = "kv-storage-${random_string.random.id}"
  resource_group_name      = azurerm_resource_group.rg.name
  location                 = azurerm_resource_group.rg_storage.location
  account_tier             = "Premium"
  account_replication_type = "LRS"
  account_kind             = "FileStorage"
}

resource "azurerm_storage_share" "FSShare" {
  name                 = "fslogix"
  storage_account_name = azurerm_storage_account.storage.name
  quota                = 5120
  depends_on           = [azurerm_storage_account.storage]
}