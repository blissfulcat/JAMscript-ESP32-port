#ifndef CNODE_H
#define CNODE_H

#include <stdbool.h>
#include "zenoh.h"
#include "core.h"
#include "system_manager.h"

/* STRUCTS & TYPEDEFS */
typedef struct _cnode_t 
{
    system_manager_t* system_manager; 
    char* node_id;
    zenoh_t* zenoh;
    corestate_t* core_state;
    bool initialized;
} cnode_t;

/* FUNCTION PROTOTYPES */

/**
 * @brief Constructor. Initiates the cnode structure and initiates all of its components. E.g., we 
 * call system_manager_init(), zenoh_init(), ...
 * @param argc - cmd line argument count
 * @param argv - cmd line args
 * @return pointer to cnode_t struct
*/
cnode_t*    cnode_init(int argc, char** argv);

/**
 * @brief Frees memory allocated during cnode_init()
 * @param cn - pointer to cnode_t struct
*/
void        cnode_destroy(cnode_t* cn);

/**
 * @brief Starts listening thread 
 * @param cn - pointer to cnode_t struct
*/
bool        cnode_start(cnode_t* cn);

/**
 * @brief Stops listening thread
 * @param cn - pointer to cnode_t struct
 * @todo should we also stop wifi activity? then we would have to start wifi activity in cnode_start
*/
bool        cnode_stop(cnode_t* cn);
#endif
