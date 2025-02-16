#ifndef __TBOARD_H__
#define __TBOARD_H__

#include "task.h"

/* STRUCTS & TYPEDEFS */

/**
 * @brief Structure representing the tboard itself. 
 * @note Can be accessed by various tasks (need to be careful about race conditions).
*/
typedef struct _tboard_t
{
    // NOTE: Should determine number of functions at compile time
    task_t*     tasks[MAX_TASKS];  ///< pointer to array of tasks 
    uint32_t    num_tasks; ///< number of tasks to run
    uint32_t    num_dead_tasks; ///< number of dead tasks 
    uint32_t    last_dead_task_id; ///< the id of the last dead task
    
    // not sure what these do but they were in the code on github
    SemaphoreHandle_t task_management_mutex; 
    StaticSemaphore_t task_management_mutex_data;
    // locking mechanism to avoid race conditions on the tboard, 
    // becareful about when to lock 
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
bool        tboard_start_task(tboard_t* tboard, int task_serial_id, arg_t** args);

/**
 * @brief Shutdown the tboard
 * @param tboard pointer to tboard_t struct
 */
void           tboard_shutdown(tboard_t *tboard);

#endif // __TBOARD_H__