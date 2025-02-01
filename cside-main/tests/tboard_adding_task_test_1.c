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
#include "task.h"

void app_main(void)
{
    char* name = "bruh";
    uint32_t serial_id = 0;
    argtype_t return_type = INT_TYPE;
    char* fn_argsig = "iii"; 

    /* Create a task that takes in three integer parameters */
    task_t* task = task_create(name, serial_id, return_type, fn_argsig);

    /* Imagine that we now want to run the task with the argument values (1, 2, 3) */

    /* Generate arguments */
    arg_t arg1 = {.type = INT_TYPE, .val.ival = 1};
    arg_t arg2 = {.type = INT_TYPE, .val.ival = 2};
    arg_t arg3 = {.type = INT_TYPE, .val.ival = 3};

    /* Set the args using variable arguments */
    task_set_args(task, 3, &arg1, &arg2, &arg3);

    /* Check that the args were correctly copied to task->args */
    arg_t** copy_args = task_get_args(task);
    if (copy_args != NULL) {
        printf("Read args: %d, %d, %d \r\n", copy_args[0]->val.ival, copy_args[1]->val.ival, copy_args[2]->val.ival);
    }

    /* Now we can run the task using the task board */

    // tboard_t* tboard = tboard_init();
    // tboard_register_func(tboard, task);
    // tboard_start_func(tboard, task->serial_id);
    
    /* Wait for the task to finish */
    // while(!task->has_finished) {};
    
    /* Return arguments will be placed in task->ret_arg */
    arg_t retarg = {.type = INT_TYPE, .val.ival = 0};
    task_set_return_arg(task, &retarg);

    /* Destroy task once its no longer in use */
    task_destroy(task);

    /* Loop forever */
    while (true) {
        sleep(1);
    }
}
