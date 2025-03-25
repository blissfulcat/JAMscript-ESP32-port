#include <stdio.h>
#include <zenoh-pico.h>
#include "utils.h"
#include "cnode.h"
#include "command.h"

/// simply task example
int example (int a, int b, int c) {
    return a+b+c; /* Synchronous task with return value */
}

void entry_point_example(execution_context_t* context) {
    arg_t** args = context->query_args;
    int a = args[0]->val.ival;
    int b = args[1]->val.ival;
    int c = args[2]->val.ival;
    int ret = example(a, b, c);
    context->return_arg->val.ival = ret;
    return;
}

// handles all date receive from zenoh subscriber
static void data_handler(z_loaned_sample_t* sample, void* arg) {
    z_view_string_t keystr;
    z_keyexpr_as_view_string(z_sample_keyexpr(sample), &keystr);
    z_owned_string_t value;
    z_bytes_to_string(z_sample_payload(sample), &value);

    cnode_t* cnode = (cnode_t*) arg;

    command_t *cmd = cnode_process_received_cmd(cnode, z_string_data(z_string_loan(&value)), (int) z_string_len(z_string_loan(&value)));
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
        if(!cnode_send_cmd(cnode, encoded_cmd)) {
            printf("Could not send command \r\n");
            return;
        }
        command_free(cmd);
        commadn_free(encoded_cmd);
    }
    // if the command is a response, send a request for the return value
    if(cmd->cmd == CMD_REXEC_RES) {
        printf("Received response \r\n");
    }

    z_string_drop(z_string_move(&value));
}


void app_main(void)
{
    int argc = 0;
    char** argv = NULL;

    cnode_t* cnode = cnode_init(argc, argv);
    /* Starts Zenoh publisher and subscriber */
    cnode_start(cnode);
    
    /* Init wifi */
    system_manager_t* sm = system_manager_init();
    if (!system_manager_wifi_init(sm)) {
        printf("Could not init wifi \r\n");
        exit(-1);
    }

    /* Init Zenoh session */
    zenoh_t* zn = zenoh_init();
    if (zn == NULL) {
        printf("Could not init Zenoh session \r\n");
        exit(-1);
    }

    /* Declare subscriber with data handler */ 
    if (!zenoh_declare_sub(zn, "app/**", data_handler, (void*) cnode)) {
        printf("Could not declare subscriber \r\n");
    }

    /* initialize variables for command_new */
    jamcommand_t cmdName = CMD_REXEC;  // Type of command (e.g., CMD_PING)
    int subcmd = 100;             // Subcommand identifier
    const char* fn_name = "example";  // Function name (could be empty if not needed)
    uint64_t task_id = 0;       // Unique task identifier
    const char* node_id = "node_123";  // Node identifier (empty string if not applicable)
    const char* fn_argsig = "iii";  // Function argument signature ("s" = string, "i" = int)
    
    int arg1 = 1;
    int arg2 = 2;
    int arg3 = 3;

    /* encode a REXEC message */
    command_t *encoded_cmd = command_new(cmdName, subcmd, fn_name, task_id, node_id, fn_argsig, arg1, arg2, arg3);
    printf("Encoded command: \r\n");
    command_print(encoded_cmd);
    /* send the message */
    if (!cnode_send_cmd(cnode, encoded_cmd)) {
        printf("Could not send command \r\n");
    }

    command_free(encoded_cmd);
    cnode_destroy(cnode);
}