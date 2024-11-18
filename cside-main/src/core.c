#include "core.h"
#include "esp_system.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "uuid4.h"

corestate_t* core_init(int serialnum) {
    // Initialize core
    corestate_t* cs=(corestate_t*)calloc(1,sizeof(corestate_t));
    cs->serial_num=serialnum;
    core_setup(cs);
    return cs;

    /**
 * @brief Constructor. Initiates the core. Calls core_setup() to generate serial & node ID
 * @param serialnum Serial number of the node
 * @return pointer to corestate_t struct
*/


}


void core_destroy(corestate_t *cs) {
//Frees memory allocated during core_init()
if(cs==NULL){
    return;
}
if(cs->device_id!=NULL){
    free(cs->device_id);
}
if(cs->serial_num!=NULL){
    free(&cs->serial_num);
}
free(cs);

}


void core_setup(corestate_t *cs) {
// No file system so just generating a device ID

//Create device ID and store
//Generate UUID4 for device_ID    

char buffer[UUID4_LEN];
uuid4_generate(buffer);
cs->device_id=strdup(buffer);
esp_err_t err = nvs_flash_init();

if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND){
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
}
ESP_ERROR_CHECK(err);

nvs_handle_t my_handle = cs->device_id;
err = nvs_open("storage", NVS_READWRITE, &my_handle);
if (err!=ESP_OK){
    printf("Error %s opening NVS handle \n", esp_err_to_name(err));
} else {
    // Read to find space
    int index=0;
    err=nvs_get_i32(my_handle,"index", &index);
    switch (err) {
        case ESP_OK:
            printf("Done\n");
            printf("Restart counter = %i \n", index);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            printf("The value is not initialized yet!\n");
            break;
        default:
            printf("Error (%s) reading!\n", esp_err_to_name(err));
    }

        printf("Updating restart counter in NVS ... ");
        index++;
        err = nvs_set_i32(my_handle, "index", index);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

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