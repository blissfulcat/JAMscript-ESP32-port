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
#include "utils.h"
#include "cnode.h"

void app_main(void)
{

    /* Init system */
    cnode_t* cnode = cnode_init(0, NULL);
    cnode->node_id = "uuid4_2";

    /* Start the receiving thread */
    if (!cnode_start(cnode)) {
        printf("Could not start cnode! \r\n");
    }

    /* Loop forever */
    while (true) {
        if (!zenoh_publish(cnode->zenoh, "Message from old board")) {
            printf("Could not publish! \r\n");
        }
        // printf("Published! \r\n");
        sleep(1);
    }
    
}
