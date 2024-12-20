#include <stdio.h>
#include <zenoh-pico.h>
#include "utils.h"
#include "cnode.h"
#include "string.h"
#include "core.h"

void app_main(void)
{
    int argc = 0;
    char** argv = NULL;

    cnode_t* cnode = cnode_init(argc, argv);
    cnode->node_id = "uuid4_1";
    /* Starts Zenoh publisher and subscriber */
    if (!cnode_start(cnode)) {
        printf("Could not start cnode \r\n");
    }

    while (true) {
        if (cnode->message_received) {
            //printf("Message received from new board, sending back ACK \r\n");
            zenoh_publish(cnode->zenoh, "ACK from new board");
            cnode->message_received = false;
        }
        sleep(1);
    }
}
