/** @addtogroup core
 * @{
 * @brief The core module provides the storing and retrivial (into flash) of the cnode nodeID and serialID fields.
 * It is one of the components of the @ref core module.
 */
#ifndef __CORE_H__
#define __CORE_H__

#include "utils.h"
// TODO: move discountFlake to an util section

/* STRUCTS & TYPEDEFS */

/**
 * @brief Struct representing the core state. 
*/
typedef struct _corestate_t
{
    char *device_id; ///< device ID (nodeID). This is a snowflakeID.
    int serial_num;  ///< serial ID (0, 1, ...)
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

/**
 * @brief Does the discount snowflake generation
 * @param buffer pointer to a buffer able to contain ID
 * @retval -1 error occured during generation
 * @retval 0 ID generation successful
*/
int discountflake(char *buffer);
#endif
/**
 * @}
*/