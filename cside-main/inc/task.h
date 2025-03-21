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


/**
 * @brief Structure representing one task that is to be run by tboard.
*/
typedef struct _task_t
{
    volatile bool is_running; ///< if the task is running or not
    volatile bool has_finished; ///< if the task has finished or not
    char* name; ///< string: name of the task
    uint32_t serial_id; ///< id starting at 0  
    TaskHandle_t task_handle_frtos; ///< task handle from free rtos
    arg_t* return_arg; ///< return value and type
    arg_t* args[MAX_ARGS]; ///< array of arg_t objects for the arguments 
    char* fn_argsig; ///< string representing the argument signature in compact form. i.e., "iis" => (int, int, string)
    function_stub_t entry_point; ///< function pointer; represents the entry point to the stub of this function
} task_t;

/* FUNCTION PROTOTYPES */

/**
 * @brief Constructor. Initializes the task_t struct. The task_handle, args and return_arg (value) are set to NULL.
 * @note Does not start the task.
 * @param name string describing name of function
 * @param serial_id id of the function
 * @param return_type enum value describing one of several possible return types of the function
 * @param fn_argsig string, argument signature (see task_t)
 * @param entry_point function pointer to stub of the function to be run (see function_stub_t)
 * @returns pointer to initialized task_t struct
 * @retval NULL if could not allocate
*/
task_t*     task_create(char* name, uint32_t serial_id, argtype_t return_type, char* fn_argsig, function_stub_t entry_point);

/**
 * @brief Destructor. Frees memory allocated for the task_t struct.
 * @param task pointer to task_t struct
*/
void        task_destroy(task_t* task);

/**
 * @brief Returns the arguments of the task.
 * @param task pointer to task_t struct
 * @returns pointer to arguments (arg_t)
*/
arg_t**      task_get_args(task_t* task); 

/**
 * @brief Set the return argument of the task.
 * @param task pointer to task_t struct
 * @param return_arg pointer to arg_t struct
*/
void        task_set_return_arg(task_t* task, arg_t* return_arg);


/**
 * @brief Set the arguments of the task.
 * @param task pointer to task_t struct
 * @param num_args number of arguments 
 * @note This function takes in variable arguments, each of which must be an arg_t* object
*/
void        task_set_args(task_t* task, int num_args, ...);


/**
 * @brief Print out information about task to the terminal.
 * @param task pointer to task_t struct
*/
void        task_print(task_t* task);
#endif // __TASK_H__
/**
 * @}
*/