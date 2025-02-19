#include "cnode.h"
#include "command.h"
#include "tboard.h"
#include "utils.h"

#define PRINT_INIT_PROGRESS // undefine to remove the initiation messages when creating a cnode
#define CNODE_REPLY_PUB_KEYEXPR "app/replies/up"
#define CNODE_REQUEST_PUB_KEYEXPR "app/requests/up"
#define CNODE_SUB_KEYEXPR "app/**"
#define DEBUG_PRINT_MESSAGES // uncomment to print out all messages received

/* PRIVATE FUNCTIONS */
static void _cnode_data_handler(z_loaned_sample_t* sample, void* arg) {
    /* Argument should be a cnode pointer */
    cnode_t* cnode = (cnode_t*) arg;
    z_view_string_t keystr;
    z_keyexpr_as_view_string(z_sample_keyexpr(sample), &keystr);
    z_owned_string_t value;
    z_bytes_to_string(z_sample_payload(sample), &value);

    /* Do not want to print out what we send out */
    // const char* cnode_pub_ke = concat(CNODE_PUB_KEYEXPR, cnode->node_id);

    /* The code below is to avoid processing our own commands */
    // char* cnode_pub_ke = CNODE_REPLY_PUB_KEYEXPR;
    // if (strncmp(z_string_data(z_view_string_loan(&keystr)), cnode_pub_ke, strlen(cnode_pub_ke)) == 0) {
    //     z_string_drop(z_string_move(&value));
    //     // free(cnode_pub_ke);
    //     return;
    // } 
    // cnode_pub_ke = CNODE_REQUEST_PUB_KEYEXPR;
    // if (strncmp(z_string_data(z_view_string_loan(&keystr)), cnode_pub_ke, strlen(cnode_pub_ke)) == 0) {
    //     z_string_drop(z_string_move(&value));
    //     // free(cnode_pub_ke);
    //     return;
    // } 

    /* The cnode should be receiving from app/replies/down or app/requests/down */
    const char* cnode_pub_ke = concat(CNODE_PUB_KEYEXPR, cnode->node_id); 
    if (strncmp(z_string_data(z_view_string_loan(&keystr)), cnode_pub_ke, strlen(cnode_pub_ke)) == 0) {
        z_string_drop(z_string_move(&value));
        free(cnode_pub_ke);
        return;
    } 

#ifdef DEBUG_PRINT_MESSAGES
    printf(" >> [Subscriber handler] Received ('%.*s': '%.*s')\n", (int)z_string_len(z_view_string_loan(&keystr)),
           z_string_data(z_view_string_loan(&keystr)), (int)z_string_len(z_string_loan(&value)),
           z_string_data(z_string_loan(&value)));
#endif

    /* Call the new function to process the message */
    cnode_process_received_cmd(cnode, z_string_data(z_string_loan(&value)), (int) z_string_len(z_string_loan(&value)));    
    /* Cleanup */
    z_string_drop(z_string_move(&value));
    // free(cnode_pub_ke);
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

#ifdef PRINT_INIT_PROGRESS
printf("cnode %ld: creating task board ... \r\n", serial_num);
#endif
        // Start the taskboard
    // cn->tboard = tboard_create(cn, cn->args->nexecs);
    // if ( cn->tboard == NULL ) {
    //     cnode_destroy(cn);
    // }

#ifdef PRINT_INIT_PROGRESS
printf("Initiating core ... \r\n");
#endif
    cn->core_state = core_init(serial_num);
    if (cn->core_state == NULL) {
        printf("Core creation failed. \r\n");
        cnode_destroy(cn);
        return NULL;
    }
    // Do we really need the node_id field? Its already in core_state
    cn->node_id = cn->core_state->device_id; 

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

// NOTE: comment given from the previous esp32 version
// Many of the cnode start/stop/destryo commands aren't necessary to use on the esp32 as we have our
// own boot phase before the user program executes.

void cnode_destroy(cnode_t* cn) {
    if (cn == NULL) {
        return;
    }
    if (cn->system_manager != NULL)
        system_manager_destroy(cn->system_manager);
    
    if (cn->core_state != NULL)
        core_destroy(cn->core_state);

    if (cn->zenoh_pub_reply != NULL) {
        //z_drop(z_move(cn->zenoh_pub_reply->z_pub));
        free(cn->zenoh_pub_reply);
        //free(cn->zenoh_pub_reply->keyexpr);
    }

    if (cn->zenoh_pub_request != NULL) {
        //z_drop(z_move(cn->zenoh_pub_request->z_pub));
        free(cn->zenoh_pub_request);
        // free(cn->zenoh_pub_request->keyexpr);
    }

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
printf("cnode %d: declaring Zenoh pubs ... \r\n", serial_num);
#endif
    /* Allocate space for the key expressions and zenoh pub objects */
    //char* cnode_reply_pub_ke = malloc(strlen(CNODE_REPLY_PUB_KEYEXPR)*sizeof(char));
    //cnode_reply_pub_ke = CNODE_REPLY_PUB_KEYEXPR;
    cn->zenoh_pub_reply = calloc(1, sizeof(zenoh_pub_t));

    //char* cnode_request_pub_ke = malloc(strlen(CNODE_REQUEST_PUB_KEYEXPR)*sizeof(char));
    //cnode_request_pub_ke = CNODE_REQUEST_PUB_KEYEXPR;
    cn->zenoh_pub_request = calloc(1, sizeof(zenoh_pub_t));

    if (!zenoh_declare_pub(cn->zenoh, CNODE_REPLY_PUB_KEYEXPR, cn->zenoh_pub_reply)) {
        printf("Could not declare reply publisher. \r\n");
        return false;
    }
    
    if (!zenoh_declare_pub(cn->zenoh, CNODE_REQUEST_PUB_KEYEXPR, cn->zenoh_pub_request)) {
        printf("Could not declare request publisher. \r\n");
        return false;
    }
    
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
    if (zp_stop_read_task(z_loan(cn->zenoh->z_session)) < 0) {
        printf("Could not stop read task \r\n");
        return false; 
    } 
    if (zp_stop_lease_task(z_loan(cn->zenoh->z_session)) < 0) {
        printf("Could not stop lease task \r\n");
        return false;
    }

    /* Undeclare subscriber and publisher */
    if (z_undeclare_subscriber(z_move(cn->zenoh->z_sub)) < 0) {
        printf("Could not undeclare sub \r\n");
        return false;
    }

    if (z_undeclare_publisher(z_move(cn->zenoh_pub_reply->z_pub)) < 0){ 
        printf("Could not undeclare reply pub \r\n");
        return false;
    }

    if (z_undeclare_publisher(z_move(cn->zenoh_pub_request->z_pub)) < 0){ 
        printf("Could not undeclare request pub \r\n");
        return false;
    }

    /* Drop session */
    if (!z_session_is_closed(z_loan(cn->zenoh->z_session))) {
        z_session_drop(z_move(cn->zenoh->z_session));
    }

    return true;
}

bool cnode_process_received_cmd(cnode_t* cn, const char* buf, size_t buflen) {
    if (!cn || !buf || buflen <= 0) {
        fprintf(stderr, "[ERROR] Invalid input to cnode_process_message\n");
        return false;
    }

    // Decode CBOR message
#ifdef DEBUG_PRINT_MESSAGES
    printf("received buffer: %s\n", buf);
#endif
    command_t *cmd = command_from_data(NULL, buf, buflen);
    if (!cmd) {
        fprintf(stderr, "[ERROR] Failed to parse command from data\n");
        return false;
    }
#ifdef DEBUG_PRINT_MESSAGES
    printf("decoded received buffer to:\n");
    command_print(cmd);
#endif
    // TODO: start task here based on the command

    free(cmd); // TODO: I don't know if I should free cmd here or not
    return true;
}

bool cnode_send_cmd(cnode_t* cn, command_t* cmd){
    if (!cn || !cmd) {
        fprintf(stderr, "[ERROR] Invalid input to cnode_send_cmd\n");
        return false;
    }
    // Publish the command to the Zenoh network
    return zenoh_publish_encoded(cn->zenoh, (const uint8_t *)cmd->buffer, (size_t) cmd->length);
}
