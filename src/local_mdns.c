#include "local_mdns.h"

#define TAG "mDNS"

void start_mdns_service(void) {
	esp_err_t err = mdns_init();
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "mDNS Init failed: %d", err);
		return;
	}

	ESP_ERROR_CHECK(mdns_hostname_set("mecanum-car"));
	ESP_ERROR_CHECK(mdns_instance_name_set("Mecanum Wheel Car Control"));
	ESP_ERROR_CHECK(mdns_service_add("My Web Server", "_http", "_tcp", 80,
					 NULL, 0));

	ESP_LOGI(TAG, "mDNS service started");
}