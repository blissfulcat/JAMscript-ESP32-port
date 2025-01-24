#ifndef __TBOARD_H__
#define __TBOARD_H__

#include "task.h"

/* STRUCTS & TYPEDEFS */

/**
 * Structure representing the tboard itself. 
 * Can be accessed by various tasks (need to be careful about race conditions).
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
 * Constructor. Initializes the tboard structure.
 * @returns pointer to initialized tboard struct
*/
tboard_t*   tboard_create();

/**
 * Destructor. Frees memory allocated during creation of the tboard structure.
*/
void        tboard_destroy();


void        tboard_register_func(tboard_t* tboard, task_t* task);

#endif // __TBOARD_H__