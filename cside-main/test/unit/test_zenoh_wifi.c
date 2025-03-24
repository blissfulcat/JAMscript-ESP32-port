#include "unity.h"
#include "system_manager.h"
#include <nvs_flash.h>
#include <esp_err.h>

// This test function initializes NVS, handles potential errors, 
// and then initializes and tears down a system manager.
void test_zenoh_wifi(void)
{
    // Initialize NVS and handle conditions where the flash needs to be erased.
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ret = nvs_flash_erase();
        TEST_ASSERT_EQUAL(ESP_OK, ret);
        ret = nvs_flash_init();
    }
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    // Initialize the system manager and test its functions.
    system_manager_t* system_manager = system_manager_init();
    TEST_ASSERT_NOT_NULL(system_manager);
    TEST_ASSERT_TRUE(system_manager_wifi_init(system_manager));
    TEST_ASSERT_TRUE(system_manager_destroy(system_manager));
}

// Main application entry point for Unity tests.
void app_main(void)
{
    UNITY_BEGIN();                // Initialize Unity
    RUN_TEST(test_zenoh_wifi);    // Run the test case for Zenoh WiFi functionality
    UNITY_END();                  // Finalize and print test results

    // Optional: keep the task alive if necessary.
    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
