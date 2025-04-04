#include "cnode.h"
#include "command.h"
#include "tboard.h"
#include "task.h"
#include "utils.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#define PRINT_INIT_PROGRESS // undefine to remove the initiation messages when creating a cnode
#define CNODE_REPLY_PUB_KEYEXPR "app/replies/up"
#define CNODE_REQUEST_PUB_KEYEXPR "app/requests/up"
#define CNODE_SUB_KEYEXPR "app/**"
#define QUEUE_LENGTH 50 // size of task processing queue
#define ITEM_SIZE sizeof(command_t *)

// function prototypes
bool cnode_send_ack(cnode_t* cn, command_t* cmd);
bool cnode_send_response(cnode_t* cn, command_t* cmd, arg_t* retarg);
bool cnode_send_error(cnode_t* cn, command_t* cmd);

/* PRIVATE FUNCTIONS */
arg_t* _cnode_return_task(cnode_t* cn, command_t* cmd) {
    if (!cn || !cmd){
        printf("one of _cnode_return_task parameter is null");
        return NULL;
    }

    task_t *task = tboard_find_task_name(cn->tboard, cmd->fn_name);
    if (!task) return NULL;
    int task_instance_idx = task_get_instance_index(task, cmd->task_id);
    if (task_instance_idx == -1) return NULL;

    task_instance_t *task_instance = task->instances[task_instance_idx];

    // this is blocking........ open a thread for this? can potentially use one of the esp32 cores
    while (!task_instance->has_finished) {
        sleep(1);
    }
    // get return value
    arg_t *retarg = task_instance_get_return_args(task_instance);

    task_instance_destroy(task_instance);
    return retarg;
}


static bool _is_own_message(z_view_string_t* keystr, const cnode_t* cnode) {
    const char* pub_ke_reply = CNODE_REPLY_PUB_KEYEXPR;
    const char* pub_ke_request = CNODE_REQUEST_PUB_KEYEXPR;
    const char* key_data = z_string_data(z_view_string_loan(keystr));

    // ignore "app/replies/up" messages
    if (strncmp(key_data, pub_ke_reply, strlen(pub_ke_reply)) == 0) {
        return true;
    }

    // ignore "app/requests/up" messages
    if (strncmp(key_data, pub_ke_request, strlen(pub_ke_request)) == 0) {
        return true;
    }

    return false;
}

static void _cnode_data_handler(z_loaned_sample_t* sample, void* arg) {
    /* Argument should be a cnode pointer */
    cnode_t* cnode = (cnode_t*) arg;
    z_view_string_t keystr;
    z_keyexpr_as_view_string(z_sample_keyexpr(sample), &keystr);
    z_owned_string_t value;
    z_bytes_to_string(z_sample_payload(sample), &value);

    /* Do not want to print out what we send out */
    if (_is_own_message(&keystr, cnode)) {
        z_string_drop(z_string_move(&value));
        return;
    }

    // debug_log(" >> [Subscriber handler] Received ('%.*s': '%.*s')\n", (int)z_string_len(z_view_string_loan(&keystr)),
    //        z_string_data(z_view_string_loan(&keystr)), (int)z_string_len(z_string_loan(&value)),
    //        z_string_data(z_string_loan(&value)));

    /* Call the new function to process the message */
    command_t *cmd = cnode_process_received_cmd(cnode, z_string_data(z_string_loan(&value)), (int) z_string_len(z_string_loan(&value)));   
    
    z_string_drop(z_string_move(&value));
    cnode->message_received = true;

    if (cmd == NULL) {
        printf("Failed to process command\n");
        return;
    }
    /* Instead of processing here, push the command onto the queue */
    if (xQueueSendToBack(cnode->commandQueue, &cmd, (TickType_t)10) != pdPASS) {
        printf("Failed to enqueue command\n");
        command_free(cmd);
        
    }
}


void cnode_cmd_processing_task(void* pvParameters) {
    cnode_t* cn = (cnode_t*) pvParameters;
    command_t* received_cmd;
    while (1) {
        if (xQueueReceive(cn->commandQueue, &received_cmd, (TickType_t)10) == pdPASS) {
            /* Process the command based on its type */
            if (received_cmd->cmd == CMD_REXEC) {
                if (!tboard_start_task(cn->tboard, received_cmd->fn_name,
                                       received_cmd->task_id, received_cmd->args)) {
                    printf("Could not start task \r\n");
                    command_free(received_cmd);
                    cnode_send_error(cn, received_cmd);
                    continue;
                } 
                
                /* Send ack */
                if (!cnode_send_ack(cn, received_cmd)) {
                    printf("Could not send ack \r\n");
                }
                command_free(received_cmd);
            }
            else if (received_cmd->cmd == CMD_GET_REXEC_RES) {
                arg_t* retarg = _cnode_return_task(cn, received_cmd);

                if (retarg == NULL) {
                    printf("Failed to get task return value\n");
                    command_free(received_cmd);
                    cnode_send_error(cn, received_cmd);
                    continue;
                } 
                if (!cnode_send_response(cn, received_cmd, retarg)) {
                    printf("Could not send response \r\n");
                }
                command_args_free(retarg);
                command_free(received_cmd);
            }
            else{
                // if the command is unknown, send an error
                cnode_send_error(cn, received_cmd);
            }
            
        }
        vTaskDelay(1);
    }
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
    cn->tboard = tboard_create();
    if ( cn->tboard == NULL ) {
        cnode_destroy(cn);
    }

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
    /* Create the command queue */
    
    cn->commandQueue = xQueueCreate(QUEUE_LENGTH, sizeof(command_t *));
    if (cn->commandQueue == NULL) {
        printf("Failed to create command queue\n");
        cnode_destroy(cn);
        return NULL;
    }

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

    if (cn->commandQueue != NULL)
        vQueueDelete(cn->commandQueue);
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

    /* Start the command processing task */
    xTaskCreate(cnode_cmd_processing_task, "cnode_processing_task", 4096, cn, 5, NULL);

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


command_t* cnode_process_received_cmd(cnode_t* cn, const char* buf, size_t buflen) {
    if (!cn || !buf || buflen <= 0) {
        fprintf(stderr, "[ERROR] Invalid input to cnode_process_message\n");
        return NULL;
    }
    // Decode CBOR message
#ifdef DEBUG_PRINT_MESSAGES
    printf("received buffer: %s\n", buf);
#endif
    command_t *cmd = command_from_data(NULL, buf, buflen);
    if (!cmd) {
        fprintf(stderr, "[ERROR] Failed to parse command from data\n");
        return NULL;
    }

    return cmd;
}

bool cnode_send_cmd(cnode_t* cn, command_t* cmd){
    if (!cn || !cmd) {
        return false;
    }
    // Publish the command to the Zenoh network
    return zenoh_publish_encoded(cn->zenoh, cn->zenoh_pub_request, (const uint8_t *)cmd->buffer, (size_t) cmd->length);
}   

bool cnode_send_response(cnode_t* cn, command_t* cmd, arg_t* retarg) {
    if (!cn || !cmd || !retarg) {
        return false;
    }
    if (!cn->zenoh || !cn->zenoh_pub_request) {
        printf("cnode_send_ack: cn->zenoh or cn->zenoh_pub_request is NULL\n");
        return false;
    }
    jamcommand_t cmdName = CMD_REXEC_RES;
    int subcmd = cmd->subcmd;
    const char* fn_name = cmd->fn_name;
    uint64_t task_id = cmd->task_id;
    const char* node_id = cmd->node_id;
    const char* fn_argsig = cmd->fn_argsig;
    
    command_t *retcmd = command_new_using_arg(cmdName, subcmd, fn_name, task_id, node_id, fn_argsig, retarg);

    if (!retcmd) {
        printf("cnode_send_response: retcmd is NULL\n");
        return false;
    }

    sleep(1); // TODO: this sleep is necessary to ensure that messages are sent consistently. There needs to be a better method
    // Publish the command to the Zenoh network
    bool sent = zenoh_publish_encoded(cn->zenoh, cn->zenoh_pub_reply, (const uint8_t *)retcmd->buffer, (size_t) retcmd->length);

    command_free(retcmd);
    return sent;
}

bool cnode_send_error(cnode_t* cn, command_t* cmd) {
    if (!cn || !cmd) {
        printf("cnode_send_error: null cnode or cmd\n");
        return false;
    }
    if (!cn->zenoh || !cn->zenoh_pub_reply) {
        printf("cnode_send_error: cn->zenoh or cn->zenoh_pub_request is NULL\n");
        return false;
    }
    jamcommand_t cmdName = CMD_REXEC_ERR;
    int subcmd = cmd->subcmd;
    const char* fn_name = cmd->fn_name;
    uint64_t task_id = cmd->task_id;
    const char* node_id = cmd->node_id;
    const char* fn_argsig = "";
    
    command_t *retcmd = command_new(cmdName, subcmd, fn_name, task_id, node_id, fn_argsig, NULL);
    if (!retcmd) {
        printf("cnode_send_ack: retcmd is NULL\n");
        return false;
    }

    sleep(1); // TODO: this sleep is necessary to ensure that messages are sent consistently. There needs to be a better method
    // Publish the command to the Zenoh network
    bool sent = zenoh_publish_encoded(cn->zenoh, cn->zenoh_pub_reply, (const uint8_t *)retcmd->buffer, (size_t)retcmd->length);
    
    command_free(retcmd);
    return sent;
}

bool cnode_send_ack(cnode_t* cn, command_t* cmd) {
    if (!cn || !cmd) {
        printf("cnode_send_ack: null cnode or cmd\n");
        return false;
    }
    if (!cn->zenoh || !cn->zenoh_pub_reply) {
        printf("cnode_send_ack: cn->zenoh or cn->zenoh_pub_request is NULL\n");
        return false;
    }
    jamcommand_t cmdName = CMD_REXEC_ACK;
    int subcmd = cmd->subcmd;
    const char* fn_name = cmd->fn_name;
    uint64_t task_id = cmd->task_id;
    const char* node_id = cmd->node_id;
    const char* fn_argsig = "";
    
    command_t *retcmd = command_new(cmdName, subcmd, fn_name, task_id, node_id, fn_argsig, NULL);
    if (!retcmd) {
        printf("cnode_send_ack: retcmd is NULL\n");
        return false;
    }

    sleep(1); // TODO: this sleep is necessary to ensure that messages are sent consistently. There needs to be a better method
    // Publish the command to the Zenoh network
    bool sent = zenoh_publish_encoded(cn->zenoh, cn->zenoh_pub_reply, (const uint8_t *)retcmd->buffer, (size_t)retcmd->length);
    
    command_free(retcmd);
    return sent;
}