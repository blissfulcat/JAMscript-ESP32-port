#ifndef __CNODE_H__
#define __CNODE_H__

#include "zenoh.h"
#include "core.h"
#include "system_manager.h"
#include "utils.h"
#include "command.h"

/* STRUCTS & TYPEDEFS */

/* CNode arguments structure created by process_args() */
typedef struct _cnode_args_t {
    char *tags;
    int groupid;
    char *appid;
    int port;
    char *host;
    int redport;
    char *redhost;
    int snumber;
    int nexecs;
} cnode_args_t;

/* CNode type, which contains CNode substructures and taskboard */
typedef struct _cnode_t 
{
    system_manager_t* system_manager; 
    char* node_id;
    zenoh_t* zenoh;
    zenoh_pub_t* zenoh_pub_reply;        ///< This publisher is to send replies back to controller
    zenoh_pub_t* zenoh_pub_request;      ///< This publisher is to send commands to controller
    corestate_t* core_state;
    bool initialized;
    volatile bool message_received; /* Here for the sub callback */
} cnode_t;

/* FUNCTION PROTOTYPES */

/**
 * @brief Constructor. Initiates the cnode structure and initiates all of its components. E.g., we 
 * call system_manager_init(), zenoh_init(), ...
 * @param argc cmd line argument count
 * @param argv cmd line args
 * @return pointer to cnode_t struct
 * @todo What should be expected from argv? e.g. Wi-Fi SSID & Password ?
*/
cnode_t*    cnode_init(int argc, char** argv);

/**
 * @brief Frees memory allocated during cnode_init().
 * @warning cnode_stop(cn) must have been called first
 * @param cn - pointer to cnode_t struct
*/
void        cnode_destroy(cnode_t* cn);

/**
 * @brief Starts a Zenoh session, along with sub and pub. Starts listening thread.
 * @param zenoh pointer to zenoh_t struct
 * @param cn pointer to cnode_t struct
*/
bool        cnode_start(cnode_t* cn);

/**
 * @brief Stops listening thread
 * @param cn pointer to cnode_t struct
 * @retval true successfully stopped the cnode
 * @retval false unsuccessfully stopped the cnode
*/
bool        cnode_stop(cnode_t* cn);

/**
 * @brief Processes an incoming message received through Zenoh.
 * @param cnode Pointer to the cnode_t instance representing the current node.
 * @param buf Pointer to the raw character buffer containing the encoded message.
 * @param buflen Length of the buffer.
 * @return True if the command was successfully processed, false otherwise.
 */
bool        cnode_process_received_cmd(cnode_t* cn, const char* buf, size_t buflen);

/**
 * @brief Sends a command to the Zenoh network.
 * @param cnode Pointer to the cnode_t instance representing the current node.
 * @param cmd Pointer to the command_t object to be sent.
 * @return True if the command was successfully sent, false otherwise.
 */
bool        cnode_send_cmd(cnode_t* cnode, command_t* cmd);

#endif
