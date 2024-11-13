#include "cnode.h"

#define PRINT_INIT_PROGRESS // undefine to remove the initiation messages when creating a cnode

cnode_t* cnode_init(int argc, char** argv) {
    /* Dynamically allocate cn */
    cnode_t* cn = (cnode_t *)calloc(1, sizeof(cnode_t));

    /* Process args */
    // TODO: args = process_args(argc, argv);
#ifdef PRINT_INIT_PROGRESS
printf("Initiating system ... \r\n");
#endif
    /* Init system */
    cn->system_manager = system_manager_init();

    if (cn->system_manager == NULL) {
        printf("System initialization failed. \r\n");
        cnode_destroy(cn);
        return NULL;
    }

#ifdef PRINT_INIT_PROGRESS
printf("Initiating Wi-Fi ... \r\n");
#endif
    /* Init wifi */
    if (!system_manager_wifi_init(cn->system_manager)) {
        printf("Could not initiate Wi-Fi. \r\n");
        cnode_destroy(cn);
        return NULL;
    }
    
    /* Init core */
    uint32_t serial_num = 0; // serial num should be determined by args 
    cn->core_state = core_init(serial_num);

// #ifdef PRINT_INIT_PROGRESS
// printf("Initiating core ... \r\n");
// #endif
//     if (cn->core_state == NULL) {
//         printf("Core creation failed. \r\n");
//         cnode_destroy(cn);
//         return NULL;
//     }
//     // Do we really need the node_id field? Its already in core_state
//     cn->node_id = cn->core_state->device_id; 
   
    
#ifdef PRINT_INIT_PROGRESS
printf("Initiating Zenoh ... \r\n");
#endif
    /* Init Zenoh */
    cn->zenoh = zenoh_init();

    if (cn->zenoh == NULL) {
        printf("Zenoh initialization failed. \r\n");
        cnode_destroy(cn);
        return NULL;
    }

#ifdef PRINT_INIT_PROGRESS
printf("Scouting for JNodes ... \r\n");
#endif
    /* Using Zenoh to scout for JNodes */
    if (!zenoh_scout(cn->zenoh)) {
        printf("Could not find any JNodes. \r\n");
        cnode_destroy(cn);
        return NULL;
    }

#ifdef PRINT_INIT_PROGRESS
printf("cnode %lu initialized. \r\n", serial_num);
#endif
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