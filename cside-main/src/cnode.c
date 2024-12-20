#include "cnode.h"

#define PRINT_INIT_PROGRESS // undefine to remove the initiation messages when creating a cnode
#define CNODE_PUB_KEYEXPR "jamscript/cnode/"
#define CNODE_SUB_KEYEXPR "jamscript/cnode/**"

/* PRIVATE FUNCTIONS */
static void _cnode_data_handler(z_loaned_sample_t* sample, void* arg) {
    /* Argument should be a cnode pointer */
    cnode_t* cnode = (cnode_t*) arg;
    z_view_string_t keystr;
    z_keyexpr_as_view_string(z_sample_keyexpr(sample), &keystr);
    z_owned_string_t value;
    z_bytes_to_string(z_sample_payload(sample), &value);
    /* Do not want to print out what we send out */
    const char* cnode_pub_ke = concat(CNODE_PUB_KEYEXPR, cnode->node_id);
    if (strncmp(z_string_data(z_view_string_loan(&keystr)), cnode_pub_ke, strlen(cnode_pub_ke)) == 0) {
        z_string_drop(z_string_move(&value));
        return;
    } 
    printf(" >> [Subscriber handler] Received ('%.*s': '%.*s')\n", (int)z_string_len(z_view_string_loan(&keystr)),
           z_string_data(z_view_string_loan(&keystr)), (int)z_string_len(z_string_loan(&value)),
           z_string_data(z_string_loan(&value)));
    z_string_drop(z_string_move(&value));
    cnode->message_received = true; /* Indicate that we have received a message */
}

/* PUBLIC FUNCTIONS */
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
        return false;
    }

#ifdef PRINT_INIT_PROGRESS
printf("Initiating Wi-Fi ... \r\n");
#endif
    /* Init wifi */
    if (!system_manager_wifi_init(cn->system_manager)) {
        printf("Could not initiate Wi-Fi. \r\n");
        cnode_destroy(cn);
        return false;
    }
    
    /* Init core */
    uint32_t serial_num = 0; // serial num should be determined by args 

// #ifdef PRINT_INIT_PROGRESS
// printf("Initiating core ... \r\n");
// #endif
//     cn->core_state = core_init(serial_num);
//     if (cn->core_state == NULL) {
//         printf("Core creation failed. \r\n");
//         cnode_destroy(cn);
//         return NULL;
//     }
//     // Do we really need the node_id field? Its already in core_state
//     cn->node_id = cn->core_state->device_id; 

/*
TODO: Currently calling zenoh_scout() creates buggy behavior for zenoh communication
*/
// #ifdef PRINT_INIT_PROGRESS
// printf("Scouting for JNodes ... \r\n");
// #endif
//     /* Using Zenoh to scout for JNodes */
//     if (!zenoh_scout()) {
//         printf("Could not find any JNodes. \r\n");
//         //cnode_destroy(cn);
//         return false;
//     }

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
    /* Make sure we don't deref null pointer ... */
    if (cn == NULL || !cn->initialized) {
        return false;
    }
    // int serial_num = cn->core->serial_num;
    int serial_num = 0; // temporary

#ifdef PRINT_INIT_PROGRESS
printf("cnode %d: declaring Zenoh session ... \r\n", serial_num);
#endif
    cn->zenoh = zenoh_init();
    if (cn->zenoh == NULL) {
        printf("Could not open Zenoh session. \r\n");
        return false;
    }
    //cn->zenoh = zenoh; /* TODO: not sure if we need this anymore */

    zenoh_start_read_task(cn->zenoh);
    zenoh_start_lease_task(cn->zenoh);

#ifdef PRINT_INIT_PROGRESS
printf("cnode %d: declaring Zenoh pub ... \r\n", serial_num);
#endif
    const char* cnode_pub_ke = concat(CNODE_PUB_KEYEXPR, cn->node_id);
    printf("%s\r\n", cnode_pub_ke);
    if (!zenoh_declare_pub(cn->zenoh, cnode_pub_ke)) {
        printf("Could not declare publisher. \r\n");
        free(cnode_pub_ke);
        return false;
    }
    free(cnode_pub_ke);
    
#ifdef PRINT_INIT_PROGRESS
printf("cnode %d: declaring Zenoh sub ... \r\n", serial_num);
#endif
    if (!zenoh_declare_sub(cn->zenoh, CNODE_SUB_KEYEXPR, _cnode_data_handler, (void*) cn)) {
        printf("Could not declare subscriber \r\n");
        return false;
    }

#ifdef PRINT_INIT_PROGRESS
printf("cnode %d: successfully started. \r\n", serial_num);
#endif
    return true;
}

bool cnode_stop(cnode_t* cn) {
    /* Make sure we don't deref null pointer ... */
    if (cn == NULL || !cn->initialized) {
        return false;
    }
    /* Stop all tasks */
    // if (!zp_stop_read_task(z_loan(cn->zenoh->z_session))) {
    //     printf("Could not stop read task \r\n");
    //     return false; 
    // } 
    // if (!zp_stop_lease_task(z_loan(cn->zenoh->z_session))) {
    //     printf("Could not stop lease task \r\n");
    //     return false;
    // }

    /* Undeclare subscriber and publisher */
    if (!z_undeclare_subscriber(z_move(cn->zenoh->z_sub))) {
        printf("Could not undeclare sub \r\n");
        return false;
    }
    if (!z_undeclare_publisher(z_move(cn->zenoh->z_pub))){ 
        printf("Could not undeclare pub \r\n");
        return false;
    }

    /* Drop session */
    if (!z_session_is_closed(z_loan(cn->zenoh->z_session))) {
        z_session_drop(z_move(cn->zenoh->z_session));
    }
    return true;
}