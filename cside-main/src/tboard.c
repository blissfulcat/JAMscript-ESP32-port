#include "tboard.h"
static tboard_t* _global_tboard; // NOTE: Temp fix to be able to update tboard correctly. Ideally there is a better solutiion.

void _task_freertos_entrypoint_wrapper(void* param)
{
    
    task_t* task = (task_t*) param;
    assert(task != NULL);
    execution_context_t ctx;
    ctx.query_args = task->args;
    ctx.return_arg = task->return_arg;

    vTaskSetThreadLocalStoragePointer( NULL,  
                                       TLSTORE_TASK_PTR_IDX,     
                                       param );

    task->is_running = true;
    task->entry_point(&ctx);

    task->has_finished = true;

    assert(ctx.return_arg != NULL);

    //if(return_arg != NULL)
    //    command_args_free(return_arg);
    //task_destroy(tboard, task);
    // TODO: need to lock with semaphore before calling task_destroy()
    task->has_finished = true;
    task->is_running = false;
    if (ctx.return_arg->type != NULL_TYPE)
        task->return_arg->val = ctx.return_arg->val;
    _global_tboard->last_dead_task_id = task->serial_id;
    _global_tboard->num_dead_tasks++;
    vTaskDelete(0);
}


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

    //implement the semaphores

    // Initialize task counters
    tboard->num_tasks = 0;
    tboard->num_dead_tasks = 0;
    tboard->last_dead_task_id = 0;
    // NOTE: This is a temporary solution in order to be able to update the tboard on task execution end
    _global_tboard = tboard;
    return tboard;
}


void        tboard_destroy(tboard_t* tboard) {
    if (tboard == NULL){
        return;
    }

    //Free memory of all tasks 
    for (int i=0; i<MAX_TASKS; i++){
        if (tboard->tasks[i] != NULL){
            task_destroy(tboard->tasks[i]);
            //tboard->tasks[i] = NULL;
        }
    }
    free(tboard);
}

void        tboard_delete_last_dead_task(tboard_t* tboard){
    if (tboard->num_dead_tasks == 0) {
        return;
    }
    uint32_t target_task_id = tboard->last_dead_task_id;

    // Delete the task and replace the tasks[i] by a null pointer
    for (int i=0; i<MAX_TASKS; i++) {
        if (tboard->tasks[i]->serial_id == target_task_id){
            task_destroy(tboard->tasks[i]);
            tboard->tasks[i] = NULL;
            tboard->num_tasks--;
            tboard->num_dead_tasks--;
        }
    }
}


void        tboard_register_task(tboard_t* tboard, task_t* task) {

    if (tboard == NULL || task == NULL){
        log_error("Unitiailized tboard or task.");
        return;
    }

    // check that no task has already the same name or serial id in the tasks
    // NOTE: Maybe we should be checking that they have BOTH the same name AND same serial id
    for (int i=0; i<MAX_TASKS; i++){
        // Comparing strings need to use some kind of strcomp and not the == operator. Also did not check if the task is NULL
        if (tboard->tasks[i] != NULL && (strcmp(tboard->tasks[i]->name, task->name)==0 || tboard->tasks[i]->serial_id == task->serial_id)){
            log_error("Task with duplicate name or serial id.");
            return;  // same name or id as a task in the array
        }
    }

    // add it in an empty space in the tboard
    for (int j=0; j<MAX_TASKS; j++){
        if(tboard->tasks[j]==NULL){
            tboard->tasks[j] = task;
            break;
        }
    }

    // Update tboard parameters
    tboard->num_tasks++;
    return;
}
/* TODO: Do not need args */
bool        tboard_start_task_id(tboard_t* tboard, int task_serial_id, arg_t** args) {
   
    if (tboard == NULL){
        return false;
    }

    task_t* task_target = tboard_find_task_id(tboard, task_serial_id);

    if (task_target == NULL) {
        log_error("Could not find task");
        return false;
    }
    // Initialize the argumnets of the task 
    //task_set_args(task_target, args); //we can assume that target_task->args has been already set
    if (task_target->args != task_get_args(task_target)) {
        log_error("Task arguments not correctly set before calling start_task().")
        return false;
    }

    xTaskCreatePinnedToCore(_task_freertos_entrypoint_wrapper, 
                            task_target->name, 
                            TASK_STACK_SIZE, 
                            task_target, 
                            1,
                            &task_target->task_handle_frtos, 
                            TASK_DEFAULT_CORE);

    // arg_t* return_arg = NULL;
    // execution_context_t ctx;
    // ctx.query_args = task_target->args;
    // ctx.return_arg = &return_arg;
    
    // vTaskSetThreadLocalStoragePointer( NULL, 0, args );

    // task_target->entry_point(&ctx);
    // task_target->is_running = true;

    // Find a way to monitor that the task is still running 
    // Use the retun argument ?
    
    // tboard->num_dead_tasks ++;
    // tboard->last_dead_task_id = task_target->serial_id;

    // task_target->is_running = false;
    // task_target->has_finished = true;
    return true;
    
}

bool        tboard_start_task_name(tboard_t* tboard, char* name, arg_t** args){

    if (tboard == NULL){
        return false;
    }

    task_t* task_target = tboard_find_task_name(tboard, name);
    if (task_target == NULL) {
        log_error("Could not find task");
        return false;
    }

    if (task_target->args != task_get_args(task_target)) {
        log_error("Task arguments not correctly set before calling start_task().")
        return false;
    }

    xTaskCreatePinnedToCore(_task_freertos_entrypoint_wrapper, 
                            task_target->name, 
                            TASK_STACK_SIZE, 
                            task_target, 
                            1,
                            &task_target->task_handle_frtos, 
                            TASK_DEFAULT_CORE);
    return true;
}

task_t*     tboard_find_task_name(tboard_t* tboard, char* name){
    
    if (tboard == NULL){
        log_error("Unitialized tboard passed to tboard_find_task_name");
        return NULL;
    }

    for (int i=0; i<MAX_TASKS; i++){
        char* target_name = tboard->tasks[i]->name;
        if (strcmp(name, target_name) == 0){
            return tboard->tasks[i];
        }
    }
    return NULL;
}

task_t*     tboard_find_task_id(tboard_t* tboard, int task_serial_id){
    
    if (tboard == NULL){
        log_error("Unitialized tboard passed to tboard_find_task_id");
        return NULL; 
    }
    
    for (int i=0; i<MAX_TASKS; i++){
        if (tboard->tasks[i]->serial_id == task_serial_id){
            return tboard->tasks[i];
        }
    }
    return NULL;
}


void tboard_print_tasks(tboard_t* tboard) {
    if (tboard == NULL) {
        log_error("Uninitialized tboard.");
    }
    printf("---------- TBOARD INFO BEGIN ----------\n");
    printf("Number of tasks:             %lu\n", tboard->num_tasks);
    printf("Number of dead tasks:        %lu\n", tboard->num_dead_tasks);
    printf("Last dead task ID:           %lu\n", tboard->last_dead_task_id);
    
    for (int i = 0; i < MAX_TASKS; i++) {
        if (tboard->tasks[i] == NULL) {
            continue;
        } else {
            task_print(tboard->tasks[i]);
            //printf("---------------\n");
        }
    }
    printf("---------- TBOARD INFO END ----------\n");
}

void           tboard_shutdown(tboard_t*tboard){
    return;
}

