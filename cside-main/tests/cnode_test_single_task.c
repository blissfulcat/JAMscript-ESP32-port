#include <stdio.h>
#include <zenoh-pico.h>
#include "utils.h"
#include "cnode.h"
#include "command.h"

zenoh_t* zn;

zenoh_pub_t z_pub_reply;
zenoh_pub_t z_pub_request;
QueueHandle_t queue;

bool send_rexec_res_cmd(command_t* received_cmd);

// handles all date receive from zenoh subscriber
static void data_handler(z_loaned_sample_t* sample, void* arg) {
    z_view_string_t keystr;
    z_keyexpr_as_view_string(z_sample_keyexpr(sample), &keystr);
    z_owned_string_t value;
    z_bytes_to_string(z_sample_payload(sample), &value);
    
    printf(" >> [Subscriber handler] Received ('%.*s': '%.*s')\n", (int)z_string_len(z_view_string_loan(&keystr)),
           z_string_data(z_view_string_loan(&keystr)), (int)z_string_len(z_string_loan(&value)),
           z_string_data(z_string_loan(&value)));

    const char* key_data = z_string_data(z_view_string_loan(&keystr));
    if (strncmp(key_data, "app/replies/down", strlen("app/replies/down")) == 0 ||
        strncmp(key_data, "app/requests/down", strlen("app/requests/down")) == 0) {
        z_string_drop(z_string_move(&value));
        return;
    }

    command_t *cmd = command_from_data(NULL, z_string_data(z_string_loan(&value)), (int) z_string_len(z_string_loan(&value)));
    if (!cmd) {
        printf("Could not process command \r\n");
        return;
    }
    printf("Received command: \r\n");
    command_print(cmd);
    command_t * const p_cmd = cmd;
    xQueueSendToBack(queue, &p_cmd, (TickType_t) 10); 

    z_string_drop(z_string_move(&value));
}

void cmd_processing_task(void* pvParameters) {
    command_t* received_cmd;
    while (1) {
        if (xQueueReceive(queue, &received_cmd, (TickType_t) 10) == pdPASS) {
                // if the command is a ACK, send a request for the return value
            if(received_cmd->cmd == CMD_REXEC_ACK) {
                printf("Received ack \r\n");
                if (!send_rexec_res_cmd(received_cmd))
                    printf("Could not send command \r\n");
            }
            // if the command is a response, send a request for the return value
            if(received_cmd->cmd == CMD_REXEC_RES) {
                printf("Received task response \r\n");
                command_print(received_cmd);
                printf("task response is %d\r\n", received_cmd->args->val.ival);
            }
            command_free(received_cmd);
            vTaskDelay(1);
        }
    }
}

bool send_rexec_res_cmd(command_t* received_cmd) {
        // create a new command the request for the return value
    jamcommand_t cmdName = CMD_GET_REXEC_RES;  
    int subcmd = received_cmd->subcmd;           
    const char* fn_name = received_cmd->fn_name; 
    uint64_t task_id = received_cmd->task_id;       
    const char* node_id = received_cmd->node_id; 
    const char* fn_argsig = received_cmd->fn_argsig; 

    command_t *encoded_cmd = command_new(cmdName, subcmd, fn_name, task_id, node_id, fn_argsig);
    if (!encoded_cmd) {
        printf("Could not create new command \r\n");
        return false;
    }
    printf("Sending command \r\n");
    command_print(encoded_cmd);
    // send the request for the return value
    if(!zenoh_publish_encoded(zn, &z_pub_reply, (const uint8_t *)encoded_cmd->buffer, (size_t) encoded_cmd->length)) {
        printf("Could not send command \r\n");
        return false;
    }
    command_free(encoded_cmd);
    return true;
}

void create_zenoh(){
    /* Queue */
    queue = xQueueCreate(10, sizeof(command_t *));
        
    xTaskCreate(cmd_processing_task,
    "processing_task",
    4096,
    NULL,
    5,
    NULL);
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

    command_free(cmd);

    while(true){
        vTaskDelay(pdMS_TO_TICKS(1000));  // delay 1 second non-blocking
    }
}