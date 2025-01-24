#ifndef __TASK_H__
#define __TASK_H__

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/event_groups.h>
#include <freertos/task.h>
#include "command.h"

/* STRUCTS & TYPEDEFS */
#define MAX_ARGS 20 ///< Maximum number of arguments 
#define MAX_TASKS 20 ///< Maximum number of tasks

/**
 * Structure representing one task that is to be run by tboard.
*/
typedef struct _task_t
{
    bool is_running; ///< if the task is running or not
    char* name; ///< string: name of the task
    uint32_t serial_id; ///< id starting at 0  
    TaskHandle_t task_handle_frtos; ///< task handle from free rtos
    arg_t* return_arg; ///< return value and type
    arg_t* args[MAX_ARGS]; ///< array of arg_t objects for the arguments 
    char* fn_argsig; ///< string representing the argument signature in compact form. i.e., "iis" => (int, int, string)   
} task_t;

/* FUNCTION PROTOTYPES */

/**
 * @brief Constructor. Initializes the task_t struct. The task_handle, args and return_arg (value) are set to NULL.
 * @param name string describing name of function
 * @param serial_id id of the function
 * @param return_type enum value describing one of several possible return types of the function
 * @param fn_argsig string, argument signature (see task_t)
 * @returns pointer to initialized task_t struct
*/
task_t*     task_create(char* name, uint32_t serial_id, argtype_t return_type, char* fn_argsig);

/**
 * @brief Destructor. Frees memory allocated for the task_t struct.
*/
void        task_destroy(task_t* task);

/**
 * @brief Returns the arguments of the task.
 * @param task pointer to task_t struct
 * @returns pointer to arguments (arg_t)
*/
arg_t*      task_get_args(task_t* task); 

/**
 * @brief Set the return argument of the task.
 * @param task pointer to task_t struct
 * @param return_arg pointer to arg_t struct
*/
void        task_set_return_arg(task_t* task, arg_t* return_arg);


#endif // __TASK_H__