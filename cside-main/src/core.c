#include "core.h"



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
free(cs->device_id);
free(cs);

}


void core_setup(corestate_t *cs) {
// No file system so just generating a device ID

//Create device ID and store
//Generate UUID4 for device_ID    

//char buffer[UUID4_LEN];
//uuid4_generate(buffer);
//cs->device_id=strdup(buffer);

}