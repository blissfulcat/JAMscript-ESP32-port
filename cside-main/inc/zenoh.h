/**
 * NOTE: The following code assumes the use of zenoh-pico release version 1.0.0
 * please ensure that this is the correct version used 
*/
#ifndef __ZENOH_H__
#define __ZENOH_H__

#include <zenoh-pico.h>
#include "utils.h"

typedef struct _zenoh_t
{
    z_owned_publisher_t z_pub;
    z_owned_subscriber_t z_sub;
    z_owned_session_t z_session;
} zenoh_t;

typedef void (*zenoh_callback_t)(z_loaned_sample_t*, void*);

/* FUNCTION PROTOTYPES */

/**
 * @brief Constructor. Initializes zenoh objects and starts a Zenoh session.
 * @return pointer to unitialized zenoh_t struct
 * @todo What configuration do we want for the zenoh session? Peer to peer, client?
*/
zenoh_t* zenoh_init();

/**
 * @brief Frees memory associated with the zenoh_t struct.
 * @param zenoh pointer to zenoh_t struct
 */
void zenoh_destroy(zenoh_t* zenoh);

/**
 * @brief Scouts for JNodes. Note that JNodes must be using Zenoh.
 * @retval true If a JNode is found
 * @retval false If a JNode is not found
 * @note Can be called even before calling zenoh_init() as long as wifi has been initiated
 * @todo Checking for JNode is not implemented. Function currently always returns true
*/
bool zenoh_scout();

/**
 * @brief Declare a zenoh subscriber on a specific topic. Assign callback function.
 * @param zenoh pointer to zenoh_t struct
 * @param key_expression string describing the 'subscription topic'
 * @param callback pointer to zenoh callback function 
 * @param cb_arg pointer to argument passed to callback function
 * @retval true If subscription declaration returned without error
 * @retval false If an error occured 
*/
bool zenoh_declare_sub(zenoh_t* zenoh, const char* key_expression, zenoh_callback_t* callback, void* cb_arg);

/**
 * @brief Declare a zenoh publisher on a specific topic.
 * @param zenoh pointer to zenoh_t struct
 * @param key_expression string describing the 'subscription topic'
 * @retval true If publish declaration returned without error
 * @retval false If an error occured 
*/
bool zenoh_declare_pub(zenoh_t* zenoh, const char* key_expression);

/**
 * @brief Start the zenoh read task by calling zp_start_read_task()
 * @param zenoh pointer to zenoh_t struct
*/
void zenoh_start_read_task(zenoh_t* zenoh); // do we really need this

/**
 * @brief Start the zenoh lease task by calling zp_start_lease_task()
 * @param zenoh pointer to zenoh_t struct
*/
void zenoh_start_lease_task(zenoh_t* zenoh); // do we really need this

/**
 * @brief Publish a message over zenoh.
 * @param zenoh pointer to zenoh_t struct
 * @param message string consisting of message
 * @retval true If publish successful
 * @retval false If an error occured 
*/
bool zenoh_publish(zenoh_t* zenoh, const char* message);
#endif