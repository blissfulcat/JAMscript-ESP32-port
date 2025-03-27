#include "tboard.h"
#include "command.h"
static tboard_t* _global_tboard; // NOTE: Temp fix to be able to update tboard correctly. Ideally there is a better solutiion.

void _task_freertos_entrypoint_wrapper(void* param)
{
    
    task_instance_t* instance = (task_instance_t*) param;
    assert(instance != NULL);
    execution_context_t ctx;
    ctx.query_args = instance->args;
    ctx.return_arg = instance->return_arg;

    vTaskSetThreadLocalStoragePointer( NULL,  
                                       TLSTORE_TASK_PTR_IDX,     
                                       param );

    instance->is_running = true;
    /* Call entry point here */
    instance->parent_task->entry_point(&ctx);

    assert(ctx.return_arg != NULL);
    /* Entry point has returned */
    instance->has_finished = true;
    instance->is_running = false;
    if (ctx.return_arg->type != NULL_TYPE) instance->return_arg->val = ctx.return_arg->val;

    /* Need to use Mutex since we access shared data structure */
    if(xSemaphoreTake(_global_tboard->task_management_mutex, MUTEX_WAIT) == pdTRUE) {
        _global_tboard->last_dead_task_id = instance->serial_id;
        _global_tboard->num_dead_tasks++;
        xSemaphoreGive(_global_tboard->task_management_mutex);
    }
    // TODO: should be some way to signal if the semaphore is not taken in time without printing (printing will cause stack to be used excessively)
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
    tboard->task_management_mutex = xSemaphoreCreateMutexStatic(&tboard->task_management_mutex_data);

    // Initialize task counters
    tboard->num_tasks = 0;
    tboard->num_dead_tasks = 0;
    tboard->last_dead_task_id = 0;
    // NOTE: This is a temporary solution in order to be able to update the tboard
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


void        tboard_register_task(tboard_t* tboard, task_t* task) {

    if (tboard == NULL || task == NULL){
        log_error("Unitiailized tboard or task.");
        return;
    }

    // check that no task has already the same name in the tasks
    for (int i=0; i<MAX_TASKS; i++){
        if (tboard->tasks[i] != NULL && (strcmp(tboard->tasks[i]->name, task->name)==0)){
            log_error("Task with duplicate name");
            return;  
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

task_instance_t*    tboard_start_task(tboard_t* tboard, char* name, int task_serial_id, arg_t* args) {
    if (tboard == NULL) {
        return NULL;
    }
    /* Find target task by name */
    printf("find task name\n");
    task_t* task_target = tboard_find_task_name(tboard, name);
    printf("  find task name done\n");

    if (task_target == NULL) {
        log_error("Could not find task name");
        return NULL;
    }
    printf("create task instance\n");
    /* Try to create a new instance of the task_target using given task_serial_id */
    task_instance_t* task_target_inst = task_instance_create(task_target, task_serial_id);
    if (task_target_inst == NULL) return NULL;
    
    printf("set task instance args\n");
    /* Try to set arguments for this instance */
    if (!task_instance_set_args(task_target_inst, args)) return NULL;

    /* Create task using FreeRTOS */
    xTaskCreatePinnedToCore(_task_freertos_entrypoint_wrapper, 
                            task_target->name, 
                            TASK_STACK_SIZE, 
                            task_target_inst, 
                            1,
                            &task_target_inst->task_handle_frtos, 
                            TASK_DEFAULT_CORE);
    return task_target_inst;
}


task_t*     tboard_find_task_name(tboard_t* tboard, char* name){
    
    if (tboard == NULL){
        log_error("Unitialized tboard passed to tboard_find_task_name");
        printf("Unitialized tboard passed to tboard_find_task_name");
        return NULL;
    }

    for (int i=0; i<MAX_TASKS; i++){
        if (tboard->tasks[i] == NULL){
            continue;
        }
        char* target_name = tboard->tasks[i]->name;
        if (strcmp(name, target_name) == 0){
            return tboard->tasks[i];
        }
    }
    return NULL;
}


void tboard_print_tasks(tboard_t* tboard) {
    if (tboard == NULL) {
        log_error("Uninitialized tboard.");
        return;
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

