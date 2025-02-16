#include "tboard.h"


tboard_t*   tboard_create() {
    tboard_t* tboard = (tboard_t*)malloc(sizeof(tboard_t));

    if (tboard == NULL) {
        // Handle memory allocation failure
        return NULL;
    }

    // Initialize the task array to NULL
    for (int i = 0; i < MAX_TASKS; i++) {
        tboard->tasks[i] = NULL;
    }

    // Initialize task counters
    tboard->num_tasks = 0;
    tboard->num_dead_tasks = 0;
    tboard->last_dead_task_id = 0;
}

// In this inplementation, the function does not check that there are still tasks in the tboard
void        tboard_destroy(tboard_t* tboard) {
    if (tboard == NULL){
        return
    }

    //Free memory of all tasks 
    for (int i=0; i<MAX_TASKS, i++){
        if (tboard->tasks != NULL){
            free(tboard->tasks(i));
            tboard->tasks[i] = NULL;
        }
    }

    free(tboard);
}

void        tboard_register_task(tboard_t* tboard, task_t* task) {

    if (tboard == NULL){
        return
    }

    // check that no task has already the same name or serial id in the tasks
    for (int i=0; i<MAX_TASKS; i++){
        if (tboard->tasks[i]->name == task->name || tboard->tasks[i]->serial_id == task->serial_id){
            return  // same name or id as a task in the array
        }
    }

    // add it in an empty space in the tboard
    for (int j=0; j<MAX_TASKS; j++){
        if(tboard->tasks[j]==NULL){
            tboard->tasks[j] = task;
        }
    }

    // Update tboard parameters
    tboard->num_tasks ++;

    return;
}

bool        tboard_start_task(tboard_t* tboard, int task_serial_id, arg_t** args) {
   
    if (tboard == NULL){
        return
    }

    task_t* task_target =tboard_find_task(tboard, task_serial_id);

    // Initialize the argumnets of the task 
    task_set_args(task_target, args);

    // Run the task 
        // if the task is stopped and not finished -> list as a dead task 
        // update the tboard parameters (dead tasks/num of tasks) 
    return NULL;
}

bool        tboard_start_task(tboard_t* tboard, char* name, arg_t** args){

    if (tboard == NULL){
        return
    }

    task_t* task_target =tboard_find_task(tboard, name);
    
    // Initialize the argumnets of the task 
    task_set_args(task_target, args);

    // Run the task 
        // if the task is stopped and not finished -> list as a dead task
        // update the tboard parameters (dead tasks/num of tasks) 
        
    return NULL;
}

task_t*     tboard_find_task(tboard_t* tboard, char* name){
    
    if (tboard == NULL){
        return
    }

    for (int i=0; i<MAX_TASKS; i++){
        char* target_name = tboard->tasks[i]->name;
        if (strcomp(name, target_name) == 1){
            return tboard->tasks[i];
        }
    }
    return NULL;
}

task_t*     tboard_find_task(tboard_t tboard, int task_serial_id){
    
    if (tboard == NULL){
        return
    }
    
    for (int i=0; i<MAX_TASKS; i++){
        if (tboard->tasks[i]->serial_id == task_serial_id){
            return tboard->tasks[i];
        }
    }
    return NULL;
}

int         strcomp(char* str1, char*str2){
    int i;
    while (str1[i] != '\0'){
        if(str1[i] != str2[i]){
            return 0;
        }
    }
    return 1;
}
