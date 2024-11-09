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

#define ESP_WIFI_SSID "WIFI USERNAME HERE"
#define ESP_WIFI_PASS "WIFI PASSWORD HERE"
#define ESP_MAXIMUM_RETRY 5
#define WIFI_CONNECTED_BIT BIT0
#define SHOULD_SKIP_WIFI_INIT false

static bool s_is_wifi_connected = false;
static EventGroupHandle_t s_event_group_handler;
static int s_retry_count = 0;
static bool _system_initialized = false;


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


system_manager_t* system_manager_init() {
	assert(!_system_initialized && "System Manager was initialized a second time!");

    /*
    We do not need a global static system manager. You should dynamically allocate system_manager
    with calloc. See cnode_init()
    */
   	system_manager_t* system_manager = &_global_system_manager; 
    memset(system_manager, 0, sizeof(system_manager_t));

    system_manager->_connection_attempts = 0;
    system_manager->wifi_connection = false;

    _system_manager_board_init(system_manager);

    _system_initialized = true;

    return system_manager;
}


void _system_manager_board_init(system_manager_t* system_manager)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Set WiFi in STA mode and trigger attachment
#ifndef SHOULD_SKIP_WIFI_INIT
    printf("Connecting to WiFi...");
    system_manager_wifi_init();
    while (!s_is_wifi_connected) {
        printf(".");
        sleep(1);
    }
    printf("OK!\n");
#endif

    int *context = (int *)malloc(sizeof(int));
    *context = 0;

    /* We should not scout here, this is done in the zenoh file via zenoh_scout().
    * scouting should also be done in cnode_init()
    * besides, this would not work because we have not opened a zenoh session
    * you should remove all of the lines below
    */
    z_owned_config_t config;
    z_config_default(&config);
    z_owned_closure_hello_t closure;
    z_closure_hello(&closure, callback, drop, context);
    printf("Scouting...\n");
    z_scout(z_config_move(&config), z_closure_hello_move(&closure), NULL);
}


void system_manager_destroy(system_manager_t* system_manager) {
    /* Also, free does nothing if system_manager is not dynamically allocated (using malloc, calloc, etc.) */
    free(system_manager);
    //reset pointer
    system_manager = NULL;
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

/*
* This function should return 'bool' (see header file)
*/
void system_manager_wifi_init(system_manager_t* system_manager) {
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
    /*
    handler_got_ip and handler_any_id are not defined anywhere, this raises a compile error 
    */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, handler_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, handler_any_id));
    vEventGroupDelete(s_event_group_handler);
}
