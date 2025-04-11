#include <stdio.h>
#include <zenoh-pico.h>
#include "utils.h"
#include "cnode.h"
#include "command.h"

void app_main(void)
{
    int argc = 0;
    char** argv = NULL;

    cnode_t* cnode = cnode_init(argc, argv);
    /* Starts Zenoh publisher and subscriber */
    cnode_start(cnode);
    
    /* initialize variables for command_new */
    jamcommand_t cmdName = CMD_PING;  // Type of command (e.g., CMD_PING)
    int subcmd = 100;             // Subcommand identifier
    const char* fn_name = "my_function";  // Function name (could be empty if not needed)
    uint64_t task_id = 200;       // Unique task identifier
    const char* node_id = "node_123";  // Node identifier (empty string if not applicable)
    const char* fn_argsig = "si";  // Function argument signature ("s" = string, "i" = int)
    
    const char* arg1 = "127.0.0.1"; // First argument: a string
    int arg2 = 42;                  // Second argument: an integer

    /* encode a ping message */
    command_t *encoded_cmd = command_new(cmdName, subcmd, fn_name, task_id, node_id, fn_argsig, arg1, arg2);
    printf("Encoded command: \r\n");
    command_print(encoded_cmd);
    /* send the message */
    if (!cnode_send_cmd(cnode, encoded_cmd)) {
        printf("Could not send command \r\n");
    }
    /* message decoding will happen in cnode_process_received_msg */

    command_free(encoded_cmd);
    cnode_destroy(cnode);
}