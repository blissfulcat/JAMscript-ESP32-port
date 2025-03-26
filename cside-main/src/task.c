#include "task.h"
#include "utils.h"
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

static  void    task_print_args(arg_t* args, int num_args) {
    bool anyargs = false;
    if (args == NULL) {
        printf("no arguments");
        return;
    }
    for (int i = 0; i < num_args; i++) {
        arg_t arg = args[i];
        //if (arg == NULL) continue;
        anyargs = true;
        switch(arg.type) {
            case INT_TYPE:
            printf("%d, ", arg.val.ival);
            break;
            case DOUBLE_TYPE:
            printf("%.3f, ", arg.val.dval);
            break;
            case STRING_TYPE:
            printf("\"%s\", ", arg.val.sval);
            break;
            case LONG_TYPE:
            printf("%ld, ", arg.val.lval);
            break;
            case NVOID_TYPE:
            printf("nvoid(n=%d), ", arg.val.nval->len);
            break;
            default:
            break;
        }
    }
    if (!anyargs) printf("no arguments");
} 


static  void    task_print_retval(arg_t* return_arg) {
    if (return_arg == NULL || return_arg->type == VOID_TYPE) {
        printf("no return value");
        return;
    }
    switch(return_arg->type) {
        case INT_TYPE:
        printf("%d, ", return_arg->val.ival);
        break;
        case DOUBLE_TYPE:
        printf("%.3f, ", return_arg->val.dval);
        break;
        case STRING_TYPE:
        printf("\"%s\", ", return_arg->val.sval);
        break;
        case LONG_TYPE:
        printf("%ld, ", return_arg->val.lval);
        break;
        case NVOID_TYPE:
        printf("nvoid(n=%d), ", return_arg->val.nval->len);
        break;
        default:
        break;
    }
}

/* This is dumb code but the free macro doesn't know to remove (num_args) * sizeof(arg_t) from the count so we will lose track of the correct count
if we just call free(instance->args) */
static  void   task_instance_args_destroy(task_instance_t* instance) {
    int num_args = instance->args->nargs;
    #ifdef MEMORY_DEBUG
    total_mem_usage -= (num_args-1) * sizeof(arg_t);
    free(instance->args);
    #else
    free(args); 
    #endif
    instance->args = NULL;
}

/* PUBLIC FUNCTIONS */
task_t*     task_create(char* name, argtype_t return_type, char* fn_argsig, function_stub_t entry_point) {
    /* Initialize task_t struct */
    task_t* task = calloc(1, sizeof(task_t));
    

    if (task == NULL) {
        printf("Could not allocate dynamically");
        return NULL;
    }
    task->name = name;
    task->return_type = return_type;
    task->fn_argsig = fn_argsig;
    task->entry_point = entry_point;

    /* Make sure all instances are set to NULL */
    for (int i = 0; i < MAX_INSTANCES; i++) {
        task->instances[i] = NULL;
    }
    task->num_instances = 0;
    return task;
}


task_instance_t* task_instance_create(task_t* parent_task, uint32_t serial_id) {
    if (parent_task == NULL) return NULL;

    /* Check if the maximum number of instances allowable has been reached */
    if (parent_task->num_instances >= MAX_INSTANCES) {
        log_error("Maximum number of instances per task reached");
        return NULL;
    }

    /* Initialize struct */
    task_instance_t* instance = calloc(1, sizeof(task_instance_t));
    arg_t* return_arg = calloc(1, sizeof(arg_t));
    if (instance == NULL || return_arg == NULL) {
        log_error("Could not allocate dynamically");
        return NULL;
    }
    instance->return_arg = return_arg;

    /* Create new instance of task using parent_task */
    instance->has_finished = false;
    instance->is_running = false;
    instance->return_arg->type = parent_task->return_type;
    instance->task_handle_frtos = NULL;
    instance->serial_id = serial_id;
    instance->parent_task = parent_task;
    instance->args = NULL;

    /* Set this instance in parent_task, find first non null entry */
    for (int i = 0; i < MAX_INSTANCES; i++) {
        if (parent_task->instances[i] != NULL) {
            /* If instance is not null, check if the serial ID already exists */
            if (parent_task->instances[i]->serial_id == instance->serial_id) {
                log_error("Task instance with same serial ID found when creating instance.");
                free(return_arg);
                free(instance);
                return NULL;
            } else {
                continue;
            }  
        }
        parent_task->instances[i] = instance;
        break;
    }
    parent_task->num_instances++;
    return instance;
}


void        task_destroy(task_t* task) {
    /* FREE ALL MEMBERS THAT ARE ALLOCATED USING MALLOC, CALLOC */
    if (task == NULL) return;
    for (int i = 0; i < MAX_INSTANCES; i++) {
        if (task->instances[i] != NULL) task_instance_destroy(task->instances[i]);
    }
    free(task);
}


void        task_instance_destroy(task_instance_t* instance) {
    if (instance == NULL) return;
    instance->parent_task->num_instances--; // decrement parent task's instance counter
    if (instance->return_arg != NULL) {free(instance->return_arg);}
    if (instance->args != NULL) {task_instance_args_destroy(instance);}
    free(instance);
}


arg_t*      task_instance_get_args(task_instance_t* instance) {
    if (instance == NULL) return NULL;
    return instance->args;
}


void        task_instance_set_return_arg(task_instance_t* instance, arg_t* return_arg) {
    if (instance == NULL || instance->return_arg == NULL) return;
    /* Check that the return type matches */
    if (return_arg->type != instance->return_arg->type) {
       log_error("Return type does not match");
       return; 
    } 
    instance->return_arg->type = return_arg->type;
    instance->return_arg->val = return_arg->val;
    return;
}


task_instance_t*    task_get_instance(task_t* task, uint32_t serial_id) {
    if (task == NULL) return NULL;
    for (int i = 0; i < MAX_INSTANCES; i++) {
        if (task->instances[i] != NULL && task->instances[i]->serial_id == serial_id) {
            return task->instances[i];
        }
    }
    return NULL;
}

bool        task_instance_set_args(task_instance_t* instance, arg_t* args) {
    printf("got here");
    if (instance == NULL) return false;
    /* If args == NULL assume that the task has no arguments */
    if (args == NULL) {
        instance->args = NULL;
        return true;
    }

    int num_args = args[0].nargs;
    if (strlen(instance->parent_task->fn_argsig) != num_args || num_args >= MAX_ARGS) {
        log_error("Number of arguments passed to task_set_args() does not match fn_argsig length or is too large");
        return false;
    }

    /* Allocate space for arguments */
    if (instance->args == NULL) {
        instance->args = (arg_t*) calloc(num_args, sizeof(arg_t));
    }

    for (int i = 0; i < num_args; i++) {
        if (char_to_argtype(instance->parent_task->fn_argsig[i]) != args[i].type) {
            log_error("Incompatible type passed to task_set_args()");
            /* Clean up args */
            task_instance_args_destroy(instance);
            return false;
        }

        if (args[i].nargs != num_args) {
            log_error("Malformed arguments array. All args need to have same nargs field value");
            /* Clean up args */
            task_instance_args_destroy(instance);
            return false;
        }
        instance->args[i] = args[i];
    }
    return true;
}

// void        task_set_args_va(task_t* task, int num_args, ...) {
//     if (task == NULL) return;

//     if (strlen(task->fn_argsig) != num_args || num_args >= MAX_ARGS) {
//         printf("Number of arguments passed to task_set_args() does not match fn_argsig length or is too large (MAX: %d)\r\n", MAX_ARGS);
//         return;
//     }

//     va_list valist;
//     va_start(valist, num_args);

//     for (int i = 0; i < num_args; i++) {
//         arg_t* arg = va_arg(valist, arg_t*);
//         if (char_to_argtype(task->fn_argsig[i]) != arg->type) {
//             printf("Incompatible type passed to task_set_args_va()\r\n");
//             va_end(valist);
//             return;
//         }
//         task->args[i] = arg;
//     }
//     va_end(valist);
//     return;
// }



void        task_print(task_t* task) {
    if (task == NULL) {
        log_error("Uninitialized task given to task_print() \r\n");
        return;
    }

    printf("\r\n ---------- TASK INFO BEGIN ---------- \r\n");
    printf("task name:               %s \r\n", task->name);
    //printf("(instance) serial id:      %lu \r\n", task->serial_id);
    printf("argsig:                  ");
    for (int i = 0; i < strlen(task->fn_argsig); i++) {
        switch(task->fn_argsig[i]) {
            case 'i':
            printf("int, ");
            break;
            case 'f':
            printf("double, ");
            break;
            case 's':
            printf("string, ");
            break;
            case 'n':
            printf("nvoid, ");
            break;
        }
    }
    printf("\r\n");
    printf("return_type:             ");
    switch(task->return_type) {
        case INT_TYPE:
        printf("int \r\n");
        break;
        case DOUBLE_TYPE:
        printf("double \r\n");
        break;
        case STRING_TYPE:
        printf("string \r\n");
        break;
        case NVOID_TYPE:
        printf("nvoid \r\n");
        break;
        case LONG_TYPE:
        printf("long \r\n");
        break;
        case VOID_TYPE:
        printf("void \r\n");
        break;
        case NULL_TYPE:
        printf("null \r\n");
        break;
    }
    printf("number of instances:     %lu\r\n\r\n", task->num_instances);

    for (int i = 0; i < task->num_instances; i++) {
        task_instance_t* instance = task->instances[i];
            printf("instance id:             %lu\r\n", instance->serial_id);
        if (instance->is_running) {
            printf("is_running:              true \r\n");
        } else {
            printf("is_running:              false \r\n");
        }

        if (instance->has_finished) {
            printf("has_finished:            true \r\n");
        } else {
            printf("has_finished:            false \r\n");
        }
        printf("arguments:               ");
        task_print_args(instance->args, strlen(instance->parent_task->fn_argsig));
        printf("\r\n");

        printf("return value:            ");
        task_print_retval(instance->return_arg);
        printf("\r\n\r\n");
    }
    printf(" ---------- TASK INFO END ---------- \r\n");
    return;
}