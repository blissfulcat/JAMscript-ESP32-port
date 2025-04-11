@mainpage ESP32 Port of JAMScript

# Goals of the Project
This project focuses on porting JAMScript to the ESP32 microcontroller, a low-cost, low-power device with built-in Wi-Fi and Bluetooth capabilities. Specifically, we are to implement a set of functions (which we will refer to as system calls) which are to be called by the JAMScript runtime and are needed for the proper execution of a JAMScript program. By enabling JAMScript on the ESP32, we aim to enhance the efficiency and performance of edge computing systems, facilitating more responsive and intelligent IoT applications. The motivation to port JAMScript to the ESP32 lies in the growing demand for efficient, lightweight, and scalable solutions for edge computing and IoT applications. JAMScript, designed to enable communication between devices and organize tasks, is a powerful framework for distributed systems. However, its current implementation has been largely focused on general-purpose computing platforms. The ESP32, with its dual-core architecture, built-in Wi-Fi and Bluetooth capabilities, low power consumption, and relatively cheap cost presents an ideal target for embedded systems requiring real-time responsiveness. By porting JAMScript to the ESP32, we aim to extend its capabilities to resource-constrained environments, enabling developers to deploy smart, distributed systems directly at the edge.

# Application Example
The implementation of JAMScript on the ESP32 opens up a wide range of applications, particularly in the realm of IoT. One prominent example is in autonomous vehicles, where real-time data processing is crucial. With JAMScript running on ESP32, vehicles can communicate with each other and with infrastructure in real-time, enabling more efficient traffic control and contributing to the development of smart cities. This example illustrates the potential of integrating JAMScript with ESP32 to enhance the efficiency, responsiveness, and intelligence of various edge computing applications.

# Dependencies
- zenoh-pico release version 1.0.0 is used
- espressif/cbor version v0.6.0~1 is used

# Module Documentation 
The documentation for the structs, enums, defines, and functions of the various components associated with this project are 
shown below. 

## @ref cnode
The cnode module includes the data structure which holds all of the information about the controller (c-side) node.
It contains functions to initiate and stop the cnode, as well as to send and receive messages over the network using
the zenoh protocol. It manages tasks using the tboard component.

## @ref zenoh
The zenoh module is a wrapper of the zenoh-pico library. It is one of the components of the @ref cnode.

## @ref command
The command modules contains structures to represent a JAMScript command as well as
functions to help encode, decode commands using CBOR. 

## @ref core
The core module provides the storing and retrivial (into flash) of the cnode nodeID and serialID fields.
It is one of the components of the @ref core module.

## @ref system_manager
The system manager module provides the ESP32 system init functionality as well as the initiation of the
Wi-Fi module, which is needed for the zenoh protocol. It is one of the components of @ref cnode.

## @ref task
The task module contains the structures that hold JAMScript tasks, which are to be run via commands.

## @ref tboard
The tboard module provides a structure to manage all of the tasks which can be executed on the cnode, as well as tasks which can be
executed remotely by the cnode. It uses FreeRTOS to manage tasks. It is one of the components of @ref cnode.

## @ref nvoid
The nvoid module is simply a definition of a custom type, which stores a void pointer (a pointer to
any possible structure) and a length n (hence the name nvoid). It is one of the types which can be
passed through the JAMScript commands.