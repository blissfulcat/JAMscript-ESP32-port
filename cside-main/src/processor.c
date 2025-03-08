#include <processor.h>
#include <cnode.h>
#include <task.h>
#include <tboard.h>
#include <command.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <constants.h>
#include "endian.h"

void execute_cmd(tboard_t *tboard, function_t *f, command_t *cmd)
{
    arg_t* args = command_args_clone(cmd->args);
    task_t* task = task_create(cmd->fn_name, cmd->task_id, args, cmd->fn_argsig, f);

    // right now it will execute immediately after adding the task
    tboard_register_task(tboard, task);    
    
    task_set_args(task, cmd->args[0].nargs, &cmd->args);
}

void process_message(tboard_t *tboard, command_t *cmd)
{
    switch (cmd->cmd)
    {
    case CmdNames_PING:
        // PING command handling remains unimplemented.
        assert(0 && "unimplemented");
        return;
    case CmdNames_REXEC:
    {
        // Look up the function associated with the command's function name.
        function_t *func = tboard_find_func(tboard, cmd->fn_name);
        if (func == NULL)
        {
            printf("Couldn't find function '%s'\n", cmd->fn_name);
            return;
        }
        execute_cmd(tboard, func, cmd);
        return;
    }
    default:
        printf("Unknown Command! (%d)\n", cmd->cmd);
        return;
    }
}
