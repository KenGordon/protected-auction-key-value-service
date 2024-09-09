resource "azurerm_resource_group" "rg" {
  name     = "${var.operator}-${var.environment}-${var.region}-kv-service-rg"
  location = var.region
}