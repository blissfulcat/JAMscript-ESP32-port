#ifndef CNODE_H
#define CNODE_H

//typedef struct _jnode_record_t
//{
//  ipv4_address_t ip;
//  port_t  port;
// } jnode_record_t;

#include "zenoh.h"
#include "core.h"
#include "system_manager.h"

typedef struct _cnode_t 
{
    system_manager_t* system_manager; 
    char* node_id;
    zenoh_t* zenoh;
    corestate_t* core_state;
    // jnode_record_t jnode_records[MAX_JNODES];
    bool initialized;
} cnode_t;


cnode_t*    cnode_init(int argc, char** argv);
void        cnode_destroy(cnode_t* cn);
bool        cnode_start(cnode_t* cn);
bool        cnode_stop(cnode_t* cn);
#endif
