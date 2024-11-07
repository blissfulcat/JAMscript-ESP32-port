#ifndef SYSTEM_MANAGER_H
#define SYSTEM_MANAGER_H

#include <stdbool.h>

/* STRUCTS & TYPEDEFS */
typedef struct _system_manager_t
{
    /* data */
    int a; // TODO
} system_manager_t;

/* FUNCTION PROTOTYPES */

/**
 * @brief Constructor. Initializes the system manager. 
 * @return pointer to system_manager_t struct
*/
system_manager_t* system_manager_init();

/**
 * @brief Initializes the Wifi module and connects to a preset network.
 * @retval true If wifi initiation successful
 * @retval false If error occured during wifi init
 * @todo How to set the SSID and Password for the wifi network?
*/
bool system_manager_wifi_init(system_manager_t* sm);

#endif