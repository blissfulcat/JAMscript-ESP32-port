#include "zenoh.h"



zenoh_t *zenoh_init() {
    return NULL;
}

bool zenoh_scout(zenoh_t* zenoh) {
    return NULL;
}

bool zenoh_declare_sub(zenoh_t* zenoh, char* key_expression, zenoh_callback_t* callback) {
    return NULL;
}

bool zenoh_declare_pub(zenoh_t* zenoh, char* key_expression) {
    return NULL;
}

void zenoh_start_read_task(zenoh_t* zenoh) {
    
} 
 
void zenoh_start_lease_task(zenoh_t* zenoh) {
    
} 
 
bool zenoh_publish(zenoh_t* zenoh, char* message) {
    return NULL;
}
