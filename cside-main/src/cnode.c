#include "cnode.h"


cnode_t* cnode_init(int argc, char** argv) {
    /* Dynamically allocate cn */
    cnode_t* cn = (cnode_t *)calloc(1, sizeof(cnode_t));

    /* Process args */
    // TODO: args = process_args(argc, argv);

    /* Init system */
    cn->system_manager = system_manager_init();

    if (cn->system_manager == NULL) {
        printf("System initialization failed. \r\n");
        cnode_destroy(cn);
        return NULL;
    }
    
    /* Init core */
    uint32_t serial_num = 0; // serial num should be determined by args 
    cn->core_state = core_init(serial_num);

    if (cn->core_state == NULL) {
        printf("Core creation failed. \r\n");
        cnode_destroy(cn);
        return NULL;
    }

    cn->node_id = cn->core_state->device_id; // Do we really need the node_id field? Its already in core_state

    /* Init Zenoh */
    cn->zenoh = zenoh_init();

    if (cn->zenoh == NULL) {
        printf("Zenoh initialization failed. \r\n");
        cnode_destroy(cn);
        return NULL;
    }
    cn->initialized = true;
    return cn;
}

void cnode_destroy(cnode_t* cn) {
    if (cn == NULL) {
        return;
    }

    if (cn->system_manager != NULL)
        system_manager_destroy(cn->system_manager);
    
    if (cn->core_state != NULL)
        core_destroy(cn->core_state);

    if (cn->zenoh != NULL)
        zenoh_destroy(cn->zenoh);
        
    free(cn);
}

bool cnode_start(cnode_t* cn) {
    return NULL;
}

bool cnode_stop(cnode_t* cn) {
    return NULL;
}