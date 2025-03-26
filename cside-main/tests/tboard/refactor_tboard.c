/***********************
* Refactor tboard.c/tboard.h comprehensive tests. 
* NOTE: Prerequisite test(s): refactor_task.c
* Create tboard test
* Register tasks test
* Register duplicate task test
* Register null task test
* Find tasks by name test
* Start asynchronous task test (infinite loop)
* Start synchronous taks (w/ return value) test
* Start multiple instances of same task test 
* Destructor/memory leak test
*
* Last modified: 03/25/2025
* Version: 2
* NOTE: Make sure #define MEMORY_DEBUG is uncommented in utils.h 
* USAGE: 
1. Run the following code as the main function and check if any asserts are not met. 
2. Compare the related output.txt with the serial output of this program.
***********************/

#include "utils.h"
#include "cnode.h"
#include "task.h"
#include "tboard.h"

#define USE_START_TASK_NAME /// Uncomment this to start tasks using tboard_start_task_id() instead.

/**
 * EXAMPLE FUNCTIONS THAT WE WANT TO ADD TO TBOARD
*/
int example (int a, int b, int c) {
    return a+b+c; /* Synchronous task with return value */
}

void example_2 (char* str, float c) {
    printf("Hello from example_2: %s\n", str); /* Synchronous task without return value */
    return;
}

void inf_loop(void) {
    printf("Entered infinite loop \r\n");
    while(1) {
        vTaskDelay(1); /* This line would be added by the compiler, or taskYIELD() (cooperative multithreading), since this task is ASYNC */
    }
}

/**
 * These function stubs would be generated by the JamScript compiler based on the implementations above.
*/
void entry_point_example(execution_context_t* context) {
    arg_t* args = context->query_args;
    int a = args[0].val.ival;
    int b = args[1].val.ival;
    int c = args[2].val.ival;
    int ret = example(a, b, c);
    context->return_arg->val.ival = ret;
    return;
}

void entry_point_example_2(execution_context_t* context) {
    arg_t* args = context->query_args;
    char* str = args[0].val.sval;
    float c = args[0].val.dval;
    example_2(str, c);
    return;
}

void entry_point_inf_loop(execution_context_t* context) {
    // this function has no arguments
    inf_loop();
}

void app_main(void)
{
    /* Create the inf loop task */
    char* il_name = "inf_loop";
    argtype_t il_return_type = VOID_TYPE;
    char* il_fn_argsig = "";
    function_stub_t il_entry_point = entry_point_inf_loop;
    task_t* il_task = task_create(il_name, il_return_type, il_fn_argsig, il_entry_point);

    /* Create a task for example 1 */
    char* name = "example";
    argtype_t return_type = INT_TYPE;
    char* fn_argsig = "iii"; 
    function_stub_t entry_point = entry_point_example;
    task_t* task = task_create(name, return_type, fn_argsig, entry_point);

    /* Create a task for example 2 */
    char* name_2 = "example_2";
    argtype_t return_type_2 = VOID_TYPE;
    char* fn_argsig_2 = "sf"; 
    function_stub_t entry_point_2 = entry_point_example_2;
    task_t* task_2 = task_create(name_2, return_type_2, fn_argsig_2, entry_point_2);

    /* Create tboard */
    tboard_t* tboard = tboard_create();
    assert(tboard != NULL);
    assert(tboard->num_tasks == 0);
    assert(tboard->last_dead_task_id == 0); 
    assert(tboard->num_dead_tasks == 0);
    for (int i = 0; i < MAX_TASKS; i++) {
        assert(tboard->tasks[i] == NULL);
    }
    printf("Create tboard test passed \r\n");

    /* Register tasks to tboard */
    tboard_register_task(tboard, il_task);
    tboard_register_task(tboard, task);
    tboard_register_task(tboard, task_2);

    assert(tboard->tasks[0] == il_task);
    assert(tboard->tasks[1] == task);
    assert(tboard->tasks[2] == task_2);
    for (int i = 3; i < MAX_TASKS; i++) assert(tboard->tasks[i] == NULL);
    assert(tboard->num_tasks == 3);
    printf("Register tasks test passed \r\n");

    /* Register null, duplicate task test */
    tboard_register_task(tboard, NULL);
    tboard_register_task(tboard, task);
    assert(tboard->tasks[0] == il_task);
    assert(tboard->tasks[1] == task);
    assert(tboard->tasks[2] == task_2);
    for (int i = 3; i < MAX_TASKS; i++) assert(tboard->tasks[i] == NULL);
    assert(tboard->num_tasks == 3);
    printf("Register null, duplicate task test passed \r\n");

    /* Find task by name test */
    assert(tboard_find_task_name(tboard, "inf_loop") == il_task);
    assert(tboard_find_task_name(tboard, "example") == task);
    assert(tboard_find_task_name(tboard, "example_2") == task_2);
    printf("Find tasks by name test passed \r\n");

    /* Starting single task instance test */
    task_instance_t* il_task_inst = tboard_start_task(tboard, "inf_loop", 0, NULL);
    assert(il_task_inst != NULL);
    assert(tboard->tasks[0]->instances[0] == il_task_inst);
    sleep(1); // wait a bit before evaluating
    assert(il_task_inst->has_finished == false);
    assert(il_task_inst->is_running == true);
    printf("Starting infinite loop task test passed \r\n");

    /* Starting task example 1 and waiting for return value */
    arg_t arg1 = {.nargs = 3, .type = INT_TYPE, .val.ival = 1};
    arg_t arg2 = {.nargs = 3, .type = INT_TYPE, .val.ival = 2};
    arg_t arg3 = {.nargs = 3, .type = INT_TYPE, .val.ival = 3};
    arg_t example_args[3] = {arg1, arg2, arg3};

    task_instance_t* task_inst = tboard_start_task(tboard, "example", 0, example_args);
    assert(task_inst != NULL);
    assert(tboard->tasks[1]->instances[0] == task_inst);

    while (!task_inst->has_finished) {
        sleep(1);
    }

    assert(task_inst->return_arg != NULL);
    assert(task_inst->return_arg->val.ival == 6);
    printf("Return value %d \r\n", task_inst->return_arg->val.ival);
    printf("Starting example 1 task test passed \r\n");

    /* Starting multiple instance of the same task */
    arg_t e2_arg1_0 = {.nargs = 2, .type = STRING_TYPE, .val.sval = "instance 0"};
    arg_t e2_arg2_0 = {.nargs = 2, .type = DOUBLE_TYPE, .val.dval = 1.0f};
    arg_t example_2_args_0[2] = {e2_arg1_0, e2_arg2_0};

    arg_t e2_arg1_1 = {.nargs = 2, .type = STRING_TYPE, .val.sval = "instance 1"};
    arg_t e2_arg2_1 = {.nargs = 2, .type = DOUBLE_TYPE, .val.dval = 2.0f};
    arg_t example_2_args_1[2] = {e2_arg1_1, e2_arg2_1};

    arg_t e2_arg1_2 = {.nargs = 2, .type = STRING_TYPE, .val.sval = "instance 2"};
    arg_t e2_arg2_2 = {.nargs = 2, .type = DOUBLE_TYPE, .val.dval = 3.0f};
    arg_t example_2_args_2[2] = {e2_arg1_2, e2_arg2_2};

    arg_t e2_arg1_3 = {.nargs = 2, .type = STRING_TYPE, .val.sval = "instance 3"};
    arg_t e2_arg2_3 = {.nargs = 2, .type = DOUBLE_TYPE, .val.dval = 69.0f};
    arg_t example_2_args_3[2] = {e2_arg1_3, e2_arg2_3};

    arg_t e2_arg1_4 = {.nargs = 2, .type = STRING_TYPE, .val.sval = "instance 4"};
    arg_t e2_arg2_4 = {.nargs = 2, .type = DOUBLE_TYPE, .val.dval = 1.420f};
    arg_t example_2_args_4[2] = {e2_arg1_4, e2_arg2_4};

    arg_t* example_2_args[5] = {example_2_args_0, example_2_args_1, example_2_args_2, example_2_args_3, example_2_args_4};

    /* Note: since arguments are not copied when passed to tboard_start_task(), we should create a new arg_t** object for each new instance */
    for (int i = 0; i < MAX_INSTANCES; i++) {
        tboard_start_task(tboard, "example_2", i, example_2_args[i]);
    }

    /* wait for tasks to complete */
    while(!tboard->tasks[2]->instances[0]->has_finished) {};
    while(!tboard->tasks[2]->instances[1]->has_finished) {};
    while(!tboard->tasks[2]->instances[2]->has_finished) {};
    while(!tboard->tasks[2]->instances[3]->has_finished) {};
    while(!tboard->tasks[2]->instances[4]->has_finished) {};
    printf("Starting multiple instances of the same task test completed \r\n");

    tboard_print_tasks(tboard);

    tboard_destroy(tboard);
    assert(total_mem_usage == 0);
    printf("Memory leak test passed \r\n");

    /* Loop forever */
    while (true) {
        sleep(1);
    }
}