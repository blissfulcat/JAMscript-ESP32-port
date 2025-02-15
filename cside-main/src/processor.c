#include <processor.h>
#include <cnode.h>
#include <task.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <constants.h>
#include "endian.h"

void execute_cmd(tboard_t *tboard, function_t *f, command_t *cmd)
{
    // This is silly for now... We don't have a persistent device id so we take the 
    // generated device id from controller.
    if(get_device_cnode()->node_id==NULL)
        get_device_cnode()->node_id = strdup(cmd->node_id);

    // Ack
    send_ack(tboard, cmd, 20);

    // Queue Task
    task_create_from_remote(tboard, f, cmd->task_id, cmd->args, true); // no return value
}

bool process_message(tboard_t *tboard, command_t *cmd)
{
    remote_task_t *rtask;
    switch (cmd->cmd)
    {
    case CMD_REXEC:
        function_t *func = tboard_find_func(tboard, cmd->fn_name);
        if (!func)
        {
            printf("Couldn't find function '%s'\n", cmd->fn_name);
            // TODO: Use Correct Error Code
            send_err(tboard, cmd, 0);
            command_free(cmd);
            return false;
        }
        execute_cmd(tboard, func, cmd);
        return true;
    default: 
        printf("Unknown Command! (%d)\n", command_to_string(cmd->cmd));
        return false;
    }
}