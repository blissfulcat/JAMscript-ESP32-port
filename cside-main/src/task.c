#include "task.h"
/* PRIVATE FUNCTIONS */
static  argtype_t    char_to_argtype(char c) {
    switch (c) {
        case 'n':
        return NVOID_TYPE;
        case 's':
        return STRING_TYPE;
        case 'i':
        return INT_TYPE;
        case 'f':
        return DOUBLE_TYPE;
    }
    return NULL_TYPE;
}


/* PUBLIC FUNCTIONS */
task_t*     task_create(char* name, uint32_t serial_id, argtype_t return_type, char* fn_argsig, function_stub_t entry_point) {
    /* Initialize task_t struct */
    task_t* task = calloc(1, sizeof(task_t));
    task->return_arg = calloc(1, sizeof(arg_t));

    if (task == NULL) {
        printf("Could not allocate dynamically");
        return NULL;
    }
    task->name = name;
    task->serial_id = serial_id;
    task->return_arg->type = return_type;
    task->fn_argsig = fn_argsig;
    task->entry_point = entry_point;
    task->is_running = false;
    return task;
}


void        task_destroy(task_t* task) {
    /* FREE ALL MEMBERS THAT ARE ALLOCATED USING MALLOC, CALLOC */
    if (task == NULL || task->return_arg == NULL) return;
    free(task->return_arg);
    free(task);
    return;
}


arg_t**      task_get_args(task_t* task) {
    if (task == NULL) return NULL;
    return task->args;
}


void        task_set_return_arg(task_t* task, arg_t* return_arg) {
    if (task == NULL || task->return_arg == NULL) return;
    /* Check that the return type matches */
    if (return_arg->type != task->return_arg->type) {
       printf("Return type does not match \r\n");
       return; 
    } 
    task->return_arg->type = return_arg->type;
    task->return_arg->val = return_arg->val;
    return;
}

void        task_set_args(task_t* task, int num_args, ...) {
    if (task == NULL) return;

    if (strlen(task->fn_argsig) != num_args || num_args >= MAX_ARGS) {
        printf("Number of arguments passed to task_set_args() does not match fn_argsig length or is too large (MAX: %d)\r\n", MAX_ARGS);
        return;
    }

    va_list valist;
    va_start(valist, num_args);

    for (int i = 0; i < num_args; i++) {
        arg_t* arg = va_arg(valist, arg_t*);
        if (char_to_argtype(task->fn_argsig[i]) != arg->type) {
            printf("Incompatible type passed to task_set_args()\r\n");
            va_end(valist);
            return;
        }
        task->args[i] = arg;
    }
    va_end(valist);
    return;
}