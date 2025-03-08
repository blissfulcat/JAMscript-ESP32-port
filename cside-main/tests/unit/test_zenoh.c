#include "unity.h"
#include "zenoh_component.h"
#include <stdio.h>
#include <zenoh-pico.h>
#include "utils.h"
#include "cnode.h"

// Test setup function
void setUp() {
    // Initialize Zenoh, if needed
#define ESP_WIFI_SSID ""
#define ESP_WIFI_PASS ""
    cnode_start(cnode);
}

// Test teardown function
void tearDown() {
    // Cleanup, if needed
    cnode_destroy(cnode);
}

// Example test case
void test_zenoh_connect() {

    int argc = 0;
    char** argv = NULL;

    cnode_t* cnode = cnode_init(argc, argv);

    int pub_result = zenoh_publish(cnode->zenoh, "Message");

    TEST_ASSERT_EQUAL(0, pub_result);
}

void app_main() {
    UNITY_BEGIN();
    RUN_TEST(test_zenoh_connect);
    UNITY_END();
}