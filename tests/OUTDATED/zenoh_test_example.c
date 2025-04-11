#include <stdio.h>
#include <zenoh-pico.h>
#include "zenoh.h"

#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/event_groups.h>
#include <freertos/task.h>
#include <nvs_flash.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ESP_WIFI_SSID "SSID"
#define ESP_WIFI_PASS "PASSWORD"

#define ESP_MAXIMUM_RETRY 5
#define WIFI_CONNECTED_BIT BIT0

static bool s_is_wifi_connected = false;
static EventGroupHandle_t s_event_group_handler;
static int s_retry_count = 0;

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_count < ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_count++;
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(s_event_group_handler, WIFI_CONNECTED_BIT);
        s_retry_count = 0;
    }
}

void wifi_init_sta(void) {
    s_event_group_handler = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&config));

    esp_event_handler_instance_t handler_any_id;
    esp_event_handler_instance_t handler_got_ip;
    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &handler_any_id));
    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &handler_got_ip));

    wifi_config_t wifi_config = {.sta = {
                                     .ssid = ESP_WIFI_SSID,
                                     .password = ESP_WIFI_PASS,
                                 }};

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    EventBits_t bits = xEventGroupWaitBits(s_event_group_handler, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        s_is_wifi_connected = true;
    }

    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, handler_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, handler_any_id));
    vEventGroupDelete(s_event_group_handler);
}

void data_handler(z_loaned_sample_t* sample, void* arg) {
    z_view_string_t keystr;
    z_keyexpr_as_view_string(z_sample_keyexpr(sample), &keystr);
    z_owned_string_t value;
    z_bytes_to_string(z_sample_payload(sample), &value);
    printf(" >> [Subscriber handler] Received ('%.*s': '%.*s')\n", (int)z_string_len(z_view_string_loan(&keystr)),
           z_string_data(z_view_string_loan(&keystr)), (int)z_string_len(z_string_loan(&value)),
           z_string_data(z_string_loan(&value)));
    z_string_drop(z_string_move(&value));
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    printf("Connecting to WiFi...");
    wifi_init_sta();
    while (!s_is_wifi_connected) {
        printf(".");
        sleep(1);
    }
    printf("WiFi Connected!\n");
    zenoh_scout();

    printf("Initiating Zenoh session ... \r\n");
    zenoh_t* zenoh = zenoh_init();
    if (zenoh == NULL) {
        printf("Could not open Zenoh session \r\n");
    } else {
        printf("Zenoh session initiated\r\n");
    }
    #define TOPIC "key/expression/**"
    // char topic[] = "key/expression/**";

    z_owned_session_t s = *(zenoh->z_session);

    printf("Declaring Subscriber on %s\r\n", TOPIC);
    if (!zenoh_declare_sub(zenoh, TOPIC, data_handler)) {
        printf("Could not declare subscriber \r\n");
    } else {
        printf("Subscriber OK!\r\n");
    }

    printf("Declaring Publisher on %s \r\n", "key/expression/example");
    
    if (!zenoh_declare_pub(zenoh, "key/expression/example")) {
        printf("Could not declare publisher \r\n");
    } else {
        printf("Publisher OK!\r\n");
    }

    const char* msg = "message over zenoh";
    printf("Attempting to publish: %s \r\n", msg);

    if (!zenoh_publish(zenoh, msg)) {
        printf("Could not publish message \r\n");
    } else {
        printf("Success! \r\n");
    }

    zenoh_destroy(zenoh);
    while (true) {
        /* Loop forever */
        sleep(1);
    }
}
