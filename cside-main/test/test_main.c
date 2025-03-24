#include "unity.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Declarations of each module’s test runner functions (which contain Unity assertions):
extern void run_publisher(void);
extern void run_zenoh_wifi(void);

// Wrap the test runners into Unity test functions
void test_run_publisher(void)
{
    run_publisher();
}

void test_run_zenoh_wifi(void)
{
    run_zenoh_wifi();
}

void app_main(void)
{
    UNITY_BEGIN();                      // Start the Unity test session

    // RUN_TEST(test_run_publisher);       // Run the publisher tests
    RUN_TEST(test_run_zenoh_wifi);      // Run the zenoh_wifi tests

    UNITY_END();                        // End the Unity test session

    // Keep the task alive if needed
    while(1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
