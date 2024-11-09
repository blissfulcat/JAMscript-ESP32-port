#ifndef __SYSTEM_MANAGER_H__
#define __SYSTEM_MANAGER_H__

#include <esp_event.h>
#include "utils.h"

/* STRUCTS & TYPEDEFS */
typedef struct _system_manager_t
{
   // add more info if needed
   int _connection_attempts;
   bool wifi_connection;

   esp_event_handler_instance_t wifi_any_event_handle;
   esp_event_handler_instance_t got_ip_event_handle;

} system_manager_t;

/* FUNCTION PROTOTYPES */

/**
 * @brief Constructor. Initializes the system manager. 
 * @return pointer to system_manager_t struct
*/
system_manager_t* system_manager_init();

/**
 * @brief Frees memory associated with the system_manager_t struct.
 * @param sm pointer to system_manager_t struct
 */
void system_manager_destroy(system_manager_t* sm);

/**
 * @brief Initializes the Wifi module and connects to a preset network.
 * @retval true If wifi initiation successful
 * @retval false If error occured during wifi init
 * @todo How to set the SSID and Password for the wifi network?
*/
bool system_manager_wifi_init(system_manager_t* sm);

#endif