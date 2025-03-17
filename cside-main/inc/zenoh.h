/** @addtogroup zenoh
 * @{
 * @brief The zenoh module is a wrapper of the zenoh-pico library. It is one of the components of the @ref cnode.
 * 
 * @note zenoh-pico version 1.0.0 is used
 */
#ifndef __ZENOH_H__
#define __ZENOH_H__

#include <zenoh-pico.h>
#include "utils.h"

/**
 * @brief Struct representing a zenoh object. 
*/
typedef struct _zenoh_t
{
    z_owned_subscriber_t z_sub; ///< zenoh subscriber instance. used when receiving messages.
    z_owned_session_t z_session; ///< zenoh session instance. 
} zenoh_t;

/**
 * @brief Struct representing a zenoh publisher.
*/
typedef struct _zenoh_pub_t
{
    z_owned_publisher_t z_pub; ///< zenoh publisher object
    char* keyexpr; ///< keyexpression (or topic) of the publisher
} zenoh_pub_t;


/**
 * @brief Function pointer typedef. Need to register this type as an argument of zenoh_declare_sub().
*/
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
 * @warning Do not use this function.
 * @todo Checking for JNode is not implemented. Function currently always returns true
 * @todo FIX this function, DO NOT USE CURRENTLY
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
 * @brief Declare a zenoh publisher on a specific topic. The resulting publisher is passed through the z_pub argument.
 * @param zenoh pointer to zenoh_t struct
 * @param key_expression string describing the 'subscription topic'
 * @param zenoh_pub pointer to zenoh_pub_t struct, which will contain the resulting z_owned_pub struct
 * @retval true If publish declaration returned without error
 * @retval false If an error occured 
*/
bool zenoh_declare_pub(zenoh_t* zenoh, const char* key_expression, zenoh_pub_t* zenoh_pub);

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
 * @brief Publish a message over zenoh using a given publisher.
 * @param zenoh pointer to zenoh_t struct
 * @param message string consisting of message
 * @param zenoh_pub pointer to zenoh_pub_t struct specifying which publisher to send over.
 * @retval true If publish successful
 * @retval false If an error occured 
*/
bool zenoh_publish(zenoh_t* zenoh, const char* message, zenoh_pub_t* zenoh_pub);
#endif


bool zenoh_publish_encoded(zenoh_t* zenoh, zenoh_pub_t* zenoh_pub, const uint8_t* buffer, size_t buffer_len);
/**
 * @}
*/