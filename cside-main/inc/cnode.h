/** @addtogroup cnode
 * @{
 * @brief The cnode module includes the data structure which holds all of the information about the controller (c-side) node.
 * It contains functions to initiate and stop the cnode, as well as to send and receive messages over the network using
 * the zenoh protocol. It manages tasks using the tboard component.
 */
#ifndef __CNODE_H__
#define __CNODE_H__

#include "zenoh.h"
#include "core.h"
#include "system_manager.h"
#include "utils.h"
#include "command.h"

/* STRUCTS & TYPEDEFS */

/** @brief arguments structure created by process_args() 
 */
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

/** @brief CNode type, which contains CNode substructures and taskboard 
 */
typedef struct _cnode_t 
{
    system_manager_t* system_manager; ///< pointer to system_manager_t object. used to initiate system & wifi
    char* node_id; ///< randomly generated (snowflakeid) ID
    zenoh_t* zenoh; ///< pointer to zenoh_t object. used to send messages over the network to other cnodes/controllers.
    corestate_t* core_state; ///< pointer to corestate_t object. used to store the node_id and serial_id in ROM.
    bool initialized; ///< boolean representing if this cnode instance has been initialized with cnode_init() or not.
    volatile bool message_received; ///< boolean representing if a message has been received, needs to be reset manually.
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
 * @brief Frees memory allocated during cnode_init()
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
 * @todo should we also stop wifi activity? then we would have to start wifi activity in cnode_start
 * @todo fix this function
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
/**
 * @}
*/
