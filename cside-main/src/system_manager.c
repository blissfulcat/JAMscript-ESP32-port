//
// Copyright (c) 2022 ZettaScale Technology
//
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// http://www.eclipse.org/legal/epl-2.0, or the Apache License, Version 2.0
// which is available at https://www.apache.org/licenses/LICENSE-2.0.
//
// SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
//
// Contributors:
//   ZettaScale Zenoh Team, <zenoh@zettascale.tech>

#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/task.h>
#include <nvs_flash.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <zenoh-pico.h>
#include "system_manager.h"


// WiFi credentials
/* TODO: how should we set the wifi credentials? */
#define ESP_WIFI_SSID "SET SSID HERE"
#define ESP_WIFI_PASS "SET PASSWORD HERE"

/// WiFi event handler
#define ESP_MAXIMUM_RETRY 5
#define WIFI_CONNECTED_BIT BIT0

static bool s_is_wifi_connected = false;
static EventGroupHandle_t s_event_group_handler;
static int s_retry_count = 0;
static bool _system_initialized = false;

/* PRIVATE FUNCTIONS */
static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
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


void _system_manager_board_init(system_manager_t* system_manager)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

/* PUBLIC FUNCTIONS */
system_manager_t* system_manager_init() {
	assert(!_system_initialized && "System Manager was initialized a second time!");
    system_manager_t* system_manager = (system_manager_t *) calloc(1, sizeof(system_manager_t));

    system_manager->_connection_attempts = 0;
    system_manager->wifi_connection = false;

    _system_manager_board_init(system_manager);

    _system_initialized = true;

    return system_manager;
}


bool system_manager_destroy(system_manager_t* system_manager) {
    if (system_manager == NULL) {
        return false;
    }
    free(system_manager);
    return true;
}


void reset_system_manager(system_manager_t* system_manager) {
    // Reset all fields to zero
    memset(system_manager, 0, sizeof(system_manager_t));
    // Reapply any specific default values
    system_manager->_connection_attempts = 0;
    system_manager->wifi_connection = false;

    // Reset event handler pointers to NULL
    system_manager->wifi_any_event_handle = NULL;
    system_manager->got_ip_event_handle = NULL;
}

bool system_manager_wifi_init(system_manager_t* system_manager) {
	s_event_group_handler = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&config));

    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, system_manager, &system_manager->wifi_any_event_handle));
    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, system_manager, &system_manager->got_ip_event_handle));

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
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, system_manager->got_ip_event_handle));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, system_manager->wifi_any_event_handle));
    vEventGroupDelete(s_event_group_handler);
    return s_is_wifi_connected;
}