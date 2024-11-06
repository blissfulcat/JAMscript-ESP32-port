#ifndef ZENOH_H
#define ZENOH_H

// include <zenoh-pico.h> 
typedef struct _zenoh_t
{
    z_owned_publisher_t* z_pub;
    z_owned_subscriber_t z_sub;
    z_owned_session_t z_session;
} zenoh_t;

typedef void (*zenoh_callback_t)(z_loaned_sample_t*, void*);

zenoh_t *zenoh_init();
bool zenoh_scout(zenoh_t* zenoh);
bool zenoh_declare_sub(zenoh_t* zenoh, char* key_expression, zenoh_callback_t* callback);
bool zenoh_declare_pub(zenoh_t* zenoh, char* key_expression);
void zenoh_start_read_task(zenoh_t* zenoh); // do we really need this
void zenoh_start_lease_task(zenoh_t* zenoh); // do we really need this
bool zenoh_publish(zenoh_t* zenoh, char* message);
#endif