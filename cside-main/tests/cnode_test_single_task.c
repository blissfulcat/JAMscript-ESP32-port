#include <stdio.h>
#include <zenoh-pico.h>
#include "utils.h"
#include "cnode.h"
#include "command.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_console.h"
volatile bool button_pressed = false;
#define INPUT_PIN 34

zenoh_t* zn;

zenoh_pub_t z_pub_reply;
zenoh_pub_t z_pub_request;
QueueHandle_t queue;

bool send_rexec_res_cmd(command_t* received_cmd);

static void IRAM_ATTR gpio_interrupt_handler(void *args)
{
    int pinNumber = (int)args;
    if (pinNumber == INPUT_PIN) {
        button_pressed = true;
    }
}

// handles all date receive from zenoh subscriber
static void data_handler(z_loaned_sample_t* sample, void* arg) {
    z_view_string_t keystr;
    z_keyexpr_as_view_string(z_sample_keyexpr(sample), &keystr);
    z_owned_string_t value;
    z_bytes_to_string(z_sample_payload(sample), &value);
    
    // printf(" >> [Subscriber handler] Received ('%.*s': '%.*s')\n", (int)z_string_len(z_view_string_loan(&keystr)),
    //        z_string_data(z_view_string_loan(&keystr)), (int)z_string_len(z_string_loan(&value)),
    //        z_string_data(z_string_loan(&value)));

    const char* key_data = z_string_data(z_view_string_loan(&keystr));
    if (strncmp(key_data, "app/replies/down", strlen("app/replies/down")) == 0 ||
        strncmp(key_data, "app/requests/down", strlen("app/requests/down")) == 0) {
        z_string_drop(z_string_move(&value));
        return;
    }

    command_t *cmd = command_from_data(NULL, z_string_data(z_string_loan(&value)), (int) z_string_len(z_string_loan(&value)));
    if (!cmd) {
        printf("Could not process command \r\n");
        return;
    }
    //printf("Received command: \r\n");
    //command_print(cmd);
    command_t * const p_cmd = cmd;
    xQueueSendToBack(queue, &p_cmd, (TickType_t) 10); 

    z_string_drop(z_string_move(&value));
}

void cmd_processing_task(void* pvParameters) {
    command_t* received_cmd;
    while (1) {
        if (xQueueReceive(queue, &received_cmd, (TickType_t) 10) == pdPASS) {
                // if the command is a ACK, send a request for the return value
            if(received_cmd->cmd == CMD_REXEC_ACK) {
                //printf("Received ack \r\n");
                if (!send_rexec_res_cmd(received_cmd))
                    log_error("Could not send command \r\n");
            }
            // if the command is a response, send a request for the return value
            if(received_cmd->cmd == CMD_REXEC_RES) {
                //printf("Received task response \r\n");
                //command_print(received_cmd);
                printf("Response from node(%s) : %d\r\n", received_cmd->node_id, received_cmd->args->val.ival);
            }
            command_free(received_cmd);
            vTaskDelay(1);
        }
    }
}

bool send_rexec_res_cmd(command_t* received_cmd) {
        // create a new command the request for the return value
    jamcommand_t cmdName = CMD_GET_REXEC_RES;  
    int subcmd = received_cmd->subcmd;           
    const char* fn_name = received_cmd->fn_name; 
    uint64_t task_id = received_cmd->task_id;       
    const char* node_id = received_cmd->node_id; 
    const char* fn_argsig = received_cmd->fn_argsig; 

    command_t *encoded_cmd = command_new(cmdName, subcmd, fn_name, task_id, node_id, fn_argsig);
    if (!encoded_cmd) {
        log_error("Could not create new command \r\n");
        return false;
    }
    //printf("Sending command \r\n");
    //command_print(encoded_cmd);
    // send the request for the return value
    if(!zenoh_publish_encoded(zn, &z_pub_reply, (const uint8_t *)encoded_cmd->buffer, (size_t) encoded_cmd->length)) {
        log_error("Could not send command \r\n");
        return false;
    }
    command_free(encoded_cmd);
    return true;
}

void create_zenoh(){
    /* Queue */
    queue = xQueueCreate(10, sizeof(command_t *));
        
    xTaskCreate(cmd_processing_task,
    "processing_task",
    4096,
    NULL,
    5,
    NULL);
    /* Init wifi */
    system_manager_t* sm = system_manager_init();
    if (!system_manager_wifi_init(sm)) {
        printf("Could not init wifi \r\n");
        exit(-1);
    }
    /* Init Zenoh session */
    zn = zenoh_init();
    if (zn == NULL) {
        printf("Could not init Zenoh session \r\n");
        exit(-1);
    }

    zenoh_start_lease_task(zn);
    zenoh_start_read_task(zn);
    
    if (!zenoh_declare_pub(zn, "app/replies/down", &z_pub_reply)) {
        printf("Could not declare pub 1 \r\n");
        exit(-1);
    }

    if (!zenoh_declare_pub(zn, "app/requests/down", &z_pub_request)) {
        printf("Could not declare pub 2 \r\n");
        exit(-1);
    }

    printf("Successfully declared 2 publishers \r\n");

    if (!zenoh_declare_sub(zn, "app/**", data_handler, NULL)) {
        printf("Could not declare subscriber \r\n");
    }
}

void getLineInput(char buf[], size_t len)
{
    memset(buf, 0, len);
    fpurge(stdin); //clears any junk in stdin
    char *bufp;
    bufp = buf;
    while(true)
        {
            vTaskDelay(100/portTICK_PERIOD_MS);
            *bufp = getchar();
            if(*bufp != '\0' && *bufp != 0xFF && *bufp != '\r') //ignores null input, 0xFF, CR in CRLF
            {
                //'enter' (EOL) handler 
                if(*bufp == '\n'){
                    *bufp = '\0';
                    break;
                } //backspace handler
                else if (*bufp == '\b'){
                    if(bufp-buf >= 1)
                        bufp--;
                }
                else{
                    //pointer to next character
                    bufp++;
                }
            }
            
            //only accept len-1 characters, (len) character being null terminator.
            if(bufp-buf > (len)-2){
                bufp = buf + (len -1);
                *bufp = '\0';
                break;
            }
        } 
}

void app_main(void)
{
    esp_rom_gpio_pad_select_gpio(INPUT_PIN);
    
    gpio_set_direction(INPUT_PIN, GPIO_MODE_INPUT);
    gpio_pulldown_en(INPUT_PIN);
    gpio_pullup_dis(INPUT_PIN);
    gpio_set_intr_type(INPUT_PIN, GPIO_INTR_POSEDGE);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(INPUT_PIN, gpio_interrupt_handler, (void *)INPUT_PIN);

    create_zenoh();

    /* initialize variables for command_new */
    jamcommand_t cmdName = CMD_REXEC;  // Type of command (e.g., CMD_PING)
    int subcmd = 100;             // Subcommand identifier
    const char* fn_name = "example";  // Function name (could be empty if not needed)
    uint64_t task_id = 200;       // Unique task identifier
    const char* node_id = "jcontroller";  // Node identifier (empty string if not applicable)
    const char* fn_argsig = "iii";  // Function argument signature ("s" = string, "i" = int)
    
    int arg1 = 1;
    int arg2 = 2;
    int arg3 = 3;

    // /* encode a REXEC message */
    // command_t *cmd = command_new(cmdName, subcmd, fn_name, task_id, node_id, fn_argsig, arg1, arg2, arg3);
    // printf("REXEC command: \r\n");
    // command_print(cmd);

    // printf("sending REXEC request\n");
    // if (!zenoh_publish_encoded(zn, &z_pub_request, (const uint8_t *)cmd->buffer, (size_t) cmd->length)) {
    //     printf("Could not send message using z_pub_request \r\n");
    // }
    // /* send the message */

    // command_free(cmd);
    int counter = 0;
    while(true) {
        if (button_pressed) {
            counter++;
            arg1 = (rand() % (20 - 0 + 1) + 0);
            arg2 = (rand() % (20 - 0 + 1) + 0);
            arg3 = (rand() % (20 - 0 + 1) + 0);
            printf("Sending request: What is %d + %d + %d ? \r\n", arg1, arg2, arg3);
            command_t *cmd_new = command_new(cmdName, subcmd, fn_name, task_id+counter, node_id, fn_argsig, arg1, arg2, arg3);
            if (!zenoh_publish_encoded(zn, &z_pub_request, (const uint8_t*) cmd_new->buffer, (size_t) cmd_new->length)) {
                log_error("Could not send message using z_pub_request");
            }   
            button_pressed = false;
            //printf("Refcount of cmd_new: %d\r\n", cmd_new->refcount);
            //command_print(cmd_new);
            command_free(cmd_new);
        }
        vTaskDelay(1);  // delay 1 second non-blocking
    }
}
