/** @addtogroup tboard
 * @{
 * @brief The tboard module provides a structure to manage all of the tasks which can be executed on the cnode, as well as tasks which can be
 * executed remotely by the cnode. It uses FreeRTOS to manage tasks. It is one of the components of @ref cnode.
 */
#ifndef __TBOARD_H__
#define __TBOARD_H__

#include "task.h"
#include "utils.h"

#define TLSTORE_TASK_PTR_IDX 0 ///< Used in _task_freertos_entrypoint_wrapper NOTE: Not sure if this is necessary but lets keep it for now
#define TASK_STACK_SIZE 2048 ///< Size of stack allocated for each running task
#define TASK_DEFAULT_CORE 1 ///< Specifies which core tasks are run on. NOTE: For now set to 1.

/* STRUCTS & TYPEDEFS */

/**
 * @brief Structure representing the tboard itself. 
 * @note Can be accessed by various tasks (need to be careful about race conditions).
*/
typedef struct _tboard_t
{
    // NOTE: Should determine number of functions at compile time
    task_t*     tasks[MAX_TASKS];                   ///< Array of task_t pointers
    uint32_t    num_tasks;                          ///< Number of tasks that have been registered
    uint32_t    num_dead_tasks;                     ///< Number of tasks that have been completed (NOTE: not 100% about this definition of 'dead')
    uint32_t    last_dead_task_id;                  ///< The ID of the last task that was declared dead
    SemaphoreHandle_t task_management_mutex;        ///< Mutex as lock to prevent race conditions between tasks
    StaticSemaphore_t task_management_mutex_data;   ///< Mutex as lock to prevent race conditions between tasks
} tboard_t;

/* FUNCTION PROTOTYPES */
/**
 * @brief Constructor. Initializes the tboard structure. Should allocate memory to the array of tasks.
 * @returns pointer to initialized tboard struct
*/
tboard_t*   tboard_create();

/**
 * @brief Destructor. Frees memory allocated during creation of the tboard structure.
 * @param tboard pointer to tboard_t struct
*/
void        tboard_destroy(tboard_t* tboard);

/**
 * @brief Delete the last dead task from the tasks array and the memory
 * @param tboard pointer to the tboard_t structure
 * @warning TODO: untested
 */
void        tboard_delete_last_dead_task(tboard_t* tboard);

/**
 * @brief Registers a task to the tboard.
 * @param tboard pointer to tboard_t struct
 * @param task pointer to task_t struct
*/
void        tboard_register_task(tboard_t* tboard, task_t* task);


/**
 * @brief Starts a task registered to the tboard with given arguments using the task serial id.
 * @note The task needs to have already been registered using tboard_register_task()
 * @param  tboard pointer to tboard_t struct
 * @param task_serial_id serial id of the task that is to be ran
 * @param args pointer to an array of arg_t pointers (each of which represents an argument)
 * @retval true if the task was started successfully
 * @retval false an error occured during task starting (i.e., task not found)
*/
bool        tboard_start_task_id(tboard_t* tboard, int task_serial_id, arg_t** args);


/**
 * @brief Starts a task registered to the tboard with given arguments using the task name.
 * @note The task needs to have already been registered using tboard_register_task()
 * @param  tboard pointer to tboard_t struct
 * @param name name of the task to run 
 * @param args pointer to an array of arg_t pointers (each of which represents an argument)
 * @retval true if the task was started successfully
 * @retval false an error occured during task starting (i.e., task not found)d
*/
bool        tboard_start_task_name(tboard_t* tboard, char* name, arg_t** args);


/* GET TASK FUNCTIONS*/

/**
 * @brief Return the task associated to name in the tboard
 * @note the task has to be registered on the tboard to be found
 * @param tboard pointer to the tboard_t structure
 * @param name char pointer to the name of the task
 * @returns pointer to the task associated with the name in the tboard
 * @returns NULL if the element cannot be found
 */
task_t*     tboard_find_task_name(tboard_t* tboard, char* name);

/**
 * @brief Return the task associated to serial ID in the tboard
 * @note the task has to be registered on the tboard to be found
 * @param tboard pointer to the tboard_t structure
 * @param name integer of the serial ID
 * @returns pointer to the task associated with the serial ID in the tboard
 * @returns NULL if the element cannot be found
 */
task_t*     tboard_find_task_id(tboard_t* tboard, int task_serial_id);


/**
 * @brief Print out tboard status as well as all current running tasks to serial.
 * @param tboard pointer to the tboard_t structure
*/
void tboard_print_tasks(tboard_t* tboard);


/**
 * @brief Shutdown the tboard
 * @param tboard pointer to tboard_t struct
 */
void           tboard_shutdown(tboard_t *tboard);

#endif // __TBOARD_H__
/**
 * @}
*/

