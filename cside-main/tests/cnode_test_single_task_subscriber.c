#include <stdio.h>
#include <zenoh-pico.h>
#include "utils.h"
#include "cnode.h"
#include "string.h"
#include "core.h"


/// simply task example
int example (int a, int b, int c) {
    return a+b+c; /* Synchronous task with return value */
}

void entry_point_example(execution_context_t* context) {
    arg_t* args = context->query_args;
    int a = args[0].val.ival;
    int b = args[1].val.ival;
    int c = args[2].val.ival;
    int ret = example(a, b, c);
    context->return_arg->val.ival = ret;
    return;
}

void create_task(cnode_t* cnode) {
    char* name = "example";
    argtype_t return_type = INT_TYPE;
    char* fn_argsig = "iii"; 
    function_stub_t entry_point = entry_point_example;
    task_t* task = task_create(name, return_type, fn_argsig, entry_point);

    tboard_register_task(cnode->tboard, task);
}


void app_main(void)
{
    int argc = 0;
    char** argv = NULL;

    cnode_t* cnode = cnode_init(argc, argv);
    cnode->node_id = "node_123";
    /* Starts Zenoh publisher and subscriber */
    if (!cnode_start(cnode)) {
        printf("Could not start cnode \r\n");
    }

    create_task(cnode);

    while (true) {
        if (cnode->message_received) {
            printf("Message received from new board, sending back ACK \r\n");
            // zenoh_publish(cnode->zenoh, "ACK from new board");
            cnode->message_received = false;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));  // delay 1 second non-blocking
    }
}
