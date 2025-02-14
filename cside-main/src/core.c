#include "core.h"
#include "esp_system.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "string.h"
#include <sys/time.h>

corestate_t* core_init(int serialnum) {
    // Initialize core
    // nvs_flash_erase();
    corestate_t* cs=(corestate_t*)calloc(1,sizeof(corestate_t));
    if (!cs){
        printf("FAILED TO INIT CS");
    }
    cs->serial_num=serialnum;
    core_setup(cs);
    return cs;


}


void core_destroy(corestate_t *cs) {
//Frees memory allocated during core_init()
if(cs==NULL){
    return;
}
if(cs->device_id!=NULL){
    free(cs->device_id);
}
free(cs);

}


void core_setup(corestate_t *cs) {
// No file system so just generating a device ID

// Generate snowflake ID for device_ID and store    
// Current iteration is **NOT** an elegant solution
// Needs to add an override system for serial and device id
char buffer[37]={""};
cs->device_id=strdup(buffer);
size_t size_of_buffer=37;

// Workaround since directly using cs->serial makes an error when loading
int32_t serial = cs->serial_num;

// Initialize flash
esp_err_t err = nvs_flash_init();

// Checking for space
if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND){
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
}
ESP_ERROR_CHECK(err);

// Open storage
nvs_handle_t my_handle;
err = nvs_open("storage", NVS_READWRITE, &my_handle);
if (err!=ESP_OK){
    printf("Error %s opening NVS handle \n", esp_err_to_name(err));
} else {
    // Checking device id and creating if needed
    err=nvs_get_str(my_handle,"device_id", cs->device_id, &size_of_buffer);
    switch (err) {
        case ESP_OK:
            printf("Done\n");
            printf("Device ID = %s \n", cs->device_id);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            printf("The value is not initialized yet!\n");
            discountflake(buffer);
            cs->device_id=strdup(buffer);
            printf("Test Device ID = %s \n", cs->device_id);
            err = nvs_set_str(my_handle, "device_id", cs->device_id);
            printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
            break;
        default:
            printf("Error (%s) reading!\n", esp_err_to_name(err));
    }
    // Checking serial number and creating if needed
    err=nvs_get_i32(my_handle,"serial_num", &serial);
    switch (err) {
        case ESP_OK:
            printf("Done\n");
            printf("Serial number = %li \n", serial);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            printf("The value is not initialized yet!\n");
            err = nvs_set_i32(my_handle, "serial_num", serial);
            printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
            cs->serial_num=serial;
            break;
        default:
            printf("Error (%s) reading!\n", esp_err_to_name(err));
    }


        // Commit written value.
        // After setting any values, nvs_commit() must be called to ensure changes are written
        // to flash storage. Implementations may write to storage at other times,
        // but this is not guaranteed.
        printf("Committing updates in NVS ... ");
        err = nvs_commit(my_handle);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
        // Close
        nvs_close(my_handle);
}

//Temporary
cs->device_id=NULL;

}

int discountflake(char *buffer){
    uint32_t errorCode=0;
    uint32_t x;
    static int counter = 0;
    counter = (counter+1) % 64; // 6 digits, max 63

    struct timeval tv_now;
    errorCode=gettimeofday(&tv_now,NULL);
    tv_now.tv_usec=tv_now.tv_usec%1024; // Cut off any bits past the 10th

    x = (uint32_t)tv_now.tv_sec * 65536 + (uint32_t)tv_now.tv_usec * 64; // first 16 bits seconds, 10 bits microseconds
    sprintf(buffer,"%li",(x+counter)); // Add in counter
    return errorCode; // Code -1: snowflake failed, Code 0: Successful
}