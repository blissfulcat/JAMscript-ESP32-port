/** @addtogroup task
 * @{
 * @brief The task module contains the structures that hold JAMScript tasks, which are to be run via commands.
 */
#ifndef __TASK_H__
#define __TASK_H__

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/event_groups.h>
#include <freertos/task.h>
#include <stdarg.h>
#include <string.h>
#include "command.h"
#include "utils.h"

/* STRUCTS & TYPEDEFS */
#define MAX_ARGS 20 ///< Maximum number of arguments 
#define MAX_TASKS 20 ///< Maximum number of tasks
#define MAX_INSTANCES 5 ///< Maximum number of instances per task

/**
 * @brief Structure containing the execution context of a currently executing task.
 */
typedef struct _execution_context_t
{
    arg_t** query_args; ///< query arguments to the task
    arg_t* return_arg; ///< return argument 
} execution_context_t;


/**
 * @brief Function pointer to a function that returns void and takes in a execution_context_t* (function_stub)
*/
typedef void (*function_stub_t)(execution_context_t*);

typedef struct _task_instance_t task_instance_t;

/**
 * @brief Structure representing one task that is to be run by tboard.
*/
typedef struct _task_t
{
    char* name; ///< string: name of the task
    argtype_t return_type; // return type
    char* fn_argsig; ///< string representing the argument signature in compact form. i.e., "iis" => (int, int, string)
    function_stub_t entry_point; ///< function pointer; represents the entry point to the stub of this function
    task_instance_t* instances[MAX_INSTANCES];  ///< array of pointers to task_instance_t structs
    uint32_t num_instances; ///< keeps track of the number of instances of this specific task
} task_t;

/**
 * @brief Structure representing instance of a task to be run by the tboard.
*/
typedef struct _task_instance_t
{
    volatile bool is_running;
    volatile bool has_finished;
    uint32_t serial_id;
    TaskHandle_t task_handle_frtos;
    arg_t* return_arg; ///< return value and type
    arg_t* args[MAX_ARGS]; ///< array of arg_t objects for the arguments 
    task_t* parent_task; ///< pointer to parent task
};


/* FUNCTION PROTOTYPES */

/**
 * @brief Constructor. Initializes the task_t struct. The task_handle, args and return_arg (value) are set to NULL.
 * @param name string describing name of function
 * @param return_type enum value describing one of several possible return types of the function
 * @param fn_argsig string, argument signature (see task_t)
 * @param entry_point function pointer to stub of the function to be run (see function_stub_t)
 * @returns pointer to initialized task_t struct
 * @retval NULL if could not allocate
*/
task_t*     task_create(char* name, argtype_t return_type, char* fn_argsig, function_stub_t entry_point);


/**
 * Constructor. Initializes an instance of the task using a given task_t struct and adds it to the parent_task array of instances.
 * Checks if there is an existing task_instance with the same serial_id.
 * @note Does not start the task instance.
 * @param parent_task pointer to task_t struct representing the parent task of the newly created task_instance_t struct.
 * @param serial_id ID that uniquely identifies this specific instance.
 * @returns pointer to initialized task_instance_t struct
 * @retval NULL if could not allocate
*/
task_instance_t*    task_instance_create(task_t* parent_task, uint32_t serial_id);


/**
 * @brief Destructor. Frees memory allocated for the task_t struct.
 * @param task pointer to task_t struct
*/
void        task_destroy(task_t* task);


/**
 * @brief Destructor. Frees memory allocated for a task_instance_t struct.
 * @param instance pointer to task_instance_t struct
*/
void        task_instance_destroy(task_instance_t* instance);


/**
 * @brief Returns the task instance with the given serial id.
 * @param task pointer to task_t struct
 * @param serial_id serial id of the task instance
 */
task_instance_t*    task_get_instance(task_t* task, uint32_t serial_id);

/**
 * @brief Returns the arguments of the task instance.
 * @param instance pointer to task_instance_t struct
 * @returns pointer to arguments (arg_t)
*/
arg_t**      task_instance_get_args(task_instance_t* instance); 

/**
 * @brief Set the return argument of the task instance.
 * @param task_instance pointer to task_instance_t struct
 * @param return_arg pointer to arg_t struct
*/
void        task_instance_set_return_arg(task_instance_t* instance, arg_t* return_arg);

/**
 * @brief Set the arguments of the task instance.
 * @param instance pointer to task_instance_t struct.
 * @param args pointer to array of pointer of arguments.
 * @param num_args number of arguments provided to task instance.
 * @retval true arguments correctly set
 * @retval false error in setting arguments
 * @warning number of arguments need to be less than MAX_ARGS 
 * @note The arguments are passed by reference and are not copied.
*/
bool        task_instance_set_args(task_instance_t* instance, arg_t** args, int num_args);

/**
 * @brief Set the arguments of the task using variable arguments.
 * @param task pointer to task_t struct
 * @param num_args number of arguments 
 * @note This function takes in variable arguments, each of which must be an arg_t* object
*/
void        task_set_args_va(task_t* task, int num_args, ...);


/**
 * @brief Print out information about task to the terminal.
 * @param task pointer to task_t struct
*/
void        task_print(task_t* task);
#endif // __TASK_H__
/**
 * @}
*/