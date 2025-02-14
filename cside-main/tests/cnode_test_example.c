#include <stdio.h>
#include <zenoh-pico.h>
#include "utils.h"
#include "cnode.h"
void app_main(void)
{
    int argc = 0;
    char** argv = NULL;

    cnode_t* cnode = cnode_init(argc, argv);
    /* Starts Zenoh publisher and subscriber */
    cnode_start(cnode);

    zenoh_publish(cnode->zenoh, "Message");
    
    cnode_destroy(cnode);
}
