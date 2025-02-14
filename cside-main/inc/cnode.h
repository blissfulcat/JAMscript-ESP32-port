#ifndef __CNODE_H__
#define __CNODE_H__

#include "zenoh.h"
#include "core.h"
#include "system_manager.h"
#include "utils.h"

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
    cnode_args_t *args;
    system_manager_t* system_manager; 
    char* node_id;
    zenoh_t* zenoh;
    corestate_t* core_state;
    bool initialized;
    volatile bool message_received; /* Here for the sub callback */
    void *tboard;
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
#endif
