#include <stdio.h>
#include <zenoh-pico.h>
#include "utils.h"
#include "cnode.h"
#include "command.h"

zenoh_t* zn;

zenoh_pub_t z_pub_reply;
zenoh_pub_t z_pub_request;


// handles all date receive from zenoh subscriber
static void data_handler(z_loaned_sample_t* sample, void* arg) {
    z_view_string_t keystr;
    z_keyexpr_as_view_string(z_sample_keyexpr(sample), &keystr);
    z_owned_string_t value;
    z_bytes_to_string(z_sample_payload(sample), &value);

    const char* key_data = z_string_data(z_view_string_loan(&keystr));
    if (strncmp(key_data, &z_pub_reply, strlen(&z_pub_reply)) == 0) {
        return;
    }

    if (strncmp(key_data, &z_pub_request, strlen(&z_pub_reply)) == 0) {
        return;
    }

    command_t *cmd = cnode_process_received_cmd(NULL, z_string_data(z_string_loan(&value)), (int) z_string_len(z_string_loan(&value)));
    if (!cmd) {
        printf("Could not process command \r\n");
        return;
    }
    printf("Received command: \r\n");
    command_print(cmd);

    // if the command is a ACK, send a request for the return value
    if(cmd->cmd == CMD_REXEC_ACK) {
        printf("Received ack \r\n");

        // create a new command teh request for the return value
        jamcommand_t cmdName = CMD_GET_REXEC_RES;  
        int subcmd = cmd->subcmd;           
        const char* fn_name = cmd->fn_name; 
        uint64_t task_id = cmd->task_id;       
        const char* node_id = cmd->node_id; 
        const char* fn_argsig = cmd->fn_argsig; 

        command_t *encoded_cmd = command_new(cmdName, subcmd, fn_name, task_id, node_id, fn_argsig);
        if (!encoded_cmd) {
            printf("Could not create new command \r\n");
            return;
        }
        // send the request for the return value
        if(!zenoh_publish_encoded(zn, &z_pub_reply, (const uint8_t *)encoded_cmd->buffer, (size_t) encoded_cmd->length)) {
            printf("Could not send command \r\n");
            return;
        }
        command_free(cmd);
        command_free(encoded_cmd);
    }
    // if the command is a response, send a request for the return value
    if(cmd->cmd == CMD_REXEC_RES) {
        printf("Received task response \r\n");
        command_print(cmd);
    }

    z_string_drop(z_string_move(&value));
}

void create_zenoh(){
    /* Init wifi */
    system_manager_t* sm = system_manager_init();
    if (!system_manager_wifi_init(sm)) {
        printf("Could not init wifi \r\n");
        exit(-1);
    }

    /* Init Zenoh session */
    zn = zenoh_init();
    if (zn == NULL) {
        printf("Could not init Zenoh session \r\n");
        exit(-1);
    }

    zenoh_start_lease_task(zn);
    zenoh_start_read_task(zn);

    if (!zenoh_declare_pub(zn, "app/replies/down", &z_pub_reply)) {
        printf("Could not declare pub 1 \r\n");
        exit(-1);
    }

    if (!zenoh_declare_pub(zn, "app/requests/down", &z_pub_request)) {
        printf("Could not declare pub 2 \r\n");
        exit(-1);
    }

    printf("Successfully declared 2 publishers \r\n");

    if (!zenoh_declare_sub(zn, "app/**", data_handler, NULL)) {
        printf("Could not declare subscriber \r\n");
    }
}

void app_main(void)
{
    create_zenoh();

    /* initialize variables for command_new */
    jamcommand_t cmdName = CMD_REXEC;  // Type of command (e.g., CMD_PING)
    int subcmd = 100;             // Subcommand identifier
    const char* fn_name = "example";  // Function name (could be empty if not needed)
    uint64_t task_id = 200;       // Unique task identifier
    const char* node_id = "node_123";  // Node identifier (empty string if not applicable)
    const char* fn_argsig = "iii";  // Function argument signature ("s" = string, "i" = int)
    
    int arg1 = 1;
    int arg2 = 2;
    int arg3 = 3;

    /* encode a REXEC message */
    command_t *cmd = command_new(cmdName, subcmd, fn_name, task_id, node_id, fn_argsig, arg1, arg2, arg3);
    printf("REXEC command: \r\n");
    command_print(cmd);

    printf("sending REXEC request\n");
    if (!zenoh_publish_encoded(zn, &z_pub_request, (const uint8_t *)cmd->buffer, (size_t) cmd->length)) {
        printf("Could not send message using z_pub_request \r\n");
    }
    /* send the message */

    // command_free(cmd);
    // cnode_destroy(cnode);

    while(true){
        sleep(1);
    }
}