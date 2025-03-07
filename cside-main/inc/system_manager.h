/** @addtogroup system_manager
 * @{
 * @brief The system manager module provides the ESP32 system init functionality as well as the initiation of the
 * Wi-Fi module, which is needed for the zenoh protocol. It is one of the components of @ref cnode.
 */
#ifndef __SYSTEM_MANAGER_H__
#define __SYSTEM_MANAGER_H__

#include <esp_event.h>
#include "utils.h"

/* STRUCTS & TYPEDEFS */
/**
 * Struct representing the system manager.
*/
typedef struct _system_manager_t
{
   // add more info if needed
   int _connection_attempts; ///< number of connections attempted
   bool wifi_connection; ///< if we are connected to the wifi or not

   esp_event_handler_instance_t wifi_any_event_handle; ///< event handle for wifi events
   esp_event_handler_instance_t got_ip_event_handle; ///< event handle for got ip event
} system_manager_t;

/* FUNCTION PROTOTYPES */

/**
 * @brief Constructor. Initializes the system manager. 
 * @return pointer to system_manager_t struct
*/
system_manager_t* system_manager_init();

/**
 * @brief Frees memory associated with the system_manager_t struct.
 * @param system_manager pointer to system_manager_t struct
 */
bool system_manager_destroy(system_manager_t* system_manager);

/**
 * @brief Initializes the Wifi module and connects to a preset network.
 * @retval true If wifi initiation successful
 * @retval false If error occured during wifi init
 * @todo How to set the SSID and Password for the wifi network?
 * @todo This function blocks the program forever if we cannot connect to the network. Maybe implement
 * a MAX_TIMEOUTS for wifi connection? Also, what happens if we do not have a wifi network to connect to?
*/
bool system_manager_wifi_init(system_manager_t* system_manager);
#endif
/**
 * @}
*/