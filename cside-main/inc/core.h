#ifndef __CORE_H__
#define __CORE_H__

#include "utils.h"

/* STRUCTS & TYPEDEFS */
typedef struct _corestate_t
{
    char *device_id;
    int serial_num;    
    // int default_port;
} corestate_t;

/* FUNCTION PROTOTYPES */

/**
 * @brief Constructor. Initiates the core. Calls core_setup() to generate serial & node ID
 * @param serialnum Serial number of the node
 * @return pointer to corestate_t struct
*/
corestate_t *core_init( int serialnum);

/**
 * @brief Frees memory allocated during core_init()
 * @param cs pointer to corestate_t struct
*/
void core_destroy(corestate_t *cs);

/**
 * @brief Does the UUID4 generation (for node ID) and stores serial & node ID into flash memory
 * @param cs pointer to corestate_t struct
*/
void core_setup(corestate_t *cs);

#endif