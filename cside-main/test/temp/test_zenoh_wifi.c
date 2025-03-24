#include <nvs_flash.h>
#include <unistd.h>
#include "system_manager.h"

void app_main() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Set WiFi in STA mode and trigger attachment
    printf("Connecting to WiFi...");
    system_manager_t* system_manager = system_manager_init();
    if (system_manager == NULL) {
        printf("Error initializing system manager\n");
        return;
    }
    bool s_is_wifi_connected = system_manager_wifi_init(system_manager);
    while (!s_is_wifi_connected) {
        printf(".");
        sleep(1);
    }
    printf("OK!\n");

    bool is_destroyed = system_manager_destroy(system_manager);
    if (is_destroyed){
        printf("deleted\n");
    }else{
        printf("not deleted\n");
    }
}