/*

The MIT License (MIT)
Copyright (c) 2016 Muthucumaru Maheswaran

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
#ifndef __COMMAND_H__
#define __COMMAND_H__

#include "nvoid.h"
#include <cbor.h>
#include <stdint.h>
#include "utils.h"

/*
 * Enumeration of command types used in the JAM protocol.
 * These represent different message types exchanged between nodes.
 */
typedef enum _jamcommand_t{
    CMD_REGISTER,
    CMD_REGISTER_ACK,
    CMD_NEW_REGISTER,
    CMD_OLD_REGISTER,
    CMD_PING,
    CMD_REXEC,
    CMD_REXEC_ACK,
    CMD_CLOSE_PORT,
    CMD_GET_REXEC_RES,
    // only most barebone commands right now
} jamcommand_t;

// NOTE: These are past commands that aren't used right now
// #define CmdNames_REGISTER 1001
// #define CmdNames_REGISTER_ACK 1002
// #define CmdNames_NEW_REGISTER 1005
// #define CmdNames_OLD_REGISTER 1006
// #define CmdNames_PING 1020
// #define CmdNames_PONG 1021
// #define CmdNames_GET_CLOUD_FOG_INFO 1100
// #define CmdNames_PUT_CLOUD_FOG_INFO 1101
// #define CmdNames_REF_CLOUD_FOG_INFO 1102
// #define CmdNames_FOG_ADD_INFO 1150
// #define CmdNames_FOG_DEL_INFO 1151
// #define CmdNames_CLOUD_ADD_INFO 1152
// #define CmdNames_CLOUD_DEL_INFO 1153
// #define CmdNames_WHERE_IS_CTRL 1550
// #define CmdNames_HERE_IS_CTRL 1551
// #define CmdNames_PROBE 2020
// #define CmdNames_PROBE_ACK 2110
// #define CmdNames_GET_SCHEDULE 3010
// #define CmdNames_PUT_SCHEDULE 3020
// #define CmdNames_REXEC 5010
// #define CmdNames_REXEC_NAK 5020
// #define CmdNames_REXEC_ACK 5030
// #define CmdNames_REXEC_RES 5040
// #define CmdNames_REXEC_ERR 5045
// #define CmdNames_REXEC_SYN 5050
// #define CmdNames_GET_REXEC_RES 5060
// #define CmdNames_COND_FALSE 5810
// #define CmdNames_FUNC_NOT_FOUND 5820
// #define CmdNames_SET_JSYS 6000
// #define CmdNames_CLOSE_PORT 6200


// #define CmdNames_STOP 7000


/*
 * Enumeration of argument types that a command can contain.
 * These define the type of each argument passed within a command.
 */
typedef enum
{
    NULL_TYPE,
    STRING_TYPE,
    INT_TYPE,
    LONG_TYPE,
    DOUBLE_TYPE,
    NVOID_TYPE,
    VOID_TYPE
} argtype_t;

/*
 * Defines length constraints for command string parameters.
 */
#define TINY_CMD_STR_LEN 16
#define SMALL_CMD_STR_LEN 32
#define LARGE_CMD_STR_LEN 128
#define HUGE_CMD_STR_LEN 1024

/*
 * Structure representing a single command argument.
 * Each argument has a type and a value stored in a union.
 */
typedef struct _arg_t {
    int nargs;      // Number of arguments
    argtype_t type; // Type of argument
    union _argvalue_t {
        int ival;
        long int lval;
        char* sval;
        double dval;
        nvoid_t* nval;
        void* vval;
    } val;
} arg_t;

/*
 * A structure to hold the outgoing and incoming command.
 * An outgoing command is parsed into a CBOR formatted byte array and
 * similarly a CBOR formatted byte array is decoded into a CBOR item handle.
 * Also, information is extracted from the CBOR item and inserted into the
 * command structure at the decoding process.
 */
typedef struct _command_t {
    jamcommand_t cmd;                             // Command type
    int subcmd;                          // Sub-command type
    char fn_name[SMALL_CMD_STR_LEN];     // Function name
    uint64_t task_id;                    // Task identifier (execution ID)
    char node_id[LARGE_CMD_STR_LEN];     // Unique node identifier (UUID4)
    char fn_argsig[SMALL_CMD_STR_LEN];   // Function argument signature
    unsigned char buffer[HUGE_CMD_STR_LEN]; // CBOR serialized data
    int length;                          // Length of CBOR data
    arg_t* args;                         // List of arguments
    int refcount;                        // Reference counter for memory management
    long id;                             // Unique command ID
} command_t;

/*
 * Structure for handling internal commands within the system.
 * A simplified command representation used for internal processing.
 */
typedef struct _internal_command_t {
    jamcommand_t cmd;         // Command type
    uint32_t task_id; // Task identifier
    arg_t* args;     // List of arguments
} internal_command_t;

/* CONSTRUCTORS */

/**
 * @brief Creates a new internal command from an existing command
 * @param cmd Pointer to an existing command_t object
 * @return Pointer to a newly allocated internal_command_t object
 */
internal_command_t* internal_command_new(command_t* cmd);

/**
 * @brief Frees an internal command
 * @param ic Pointer to the internal command to be freed
 */
void internal_command_free(internal_command_t* ic);

/**
 * @brief Creates a new command object with variable arguments
 * @param cmd Command type
 * @param subcmd Sub-command type
 * @param fn_name Function name
 * @param task_id Task identifier
 * @param node_id Node UUID
 * @param fn_argsig Argument signature
 * @return Pointer to newly allocated command object
 */
command_t* command_new(jamcommand_t cmd, int subcmd, const char* fn_name, uint64_t task_id,
                       const char* node_id, const char* fn_argsig, ...);

/**
 * @brief Creates a new command object using an argument list
 * @param cmd Command type
 * @param opt Optional parameters
 * @param fn_name Function name
 * @param taskid Task identifier
 * @param node_id Node UUID
 * @param fn_argsig Argument signature
 * @param args Pointer to argument list
 * @return Pointer to newly allocated command object
 */
command_t* command_new_using_arg(jamcommand_t cmd, int opt, const char* fn_name,
                                 uint64_t taskid, const char* node_id,
                                 const char* fn_argsig, arg_t* args);

/**
 * @brief Initializes an existing command object using arguments
 * @param command Pointer to command object
 * @param cmd Command type
 * @param opt Optional parameters
 * @param fn_name Function name
 * @param taskid Task identifier
 * @param node_id Node UUID
 * @param fn_argsig Argument signature
 * @param args Pointer to argument list
 */
void command_init_using_arg(command_t* command, jamcommand_t cmd, int opt, const char* fn_name,
                                 uint64_t taskid, const char* node_id,
                                 const char* fn_argsig, arg_t* args);

/**
 * @brief Constructs a command from raw data
 * @param fn_argsig Argument signature
 * @param data Pointer to raw data
 * @param len Length of data
 * @return Pointer to newly allocated command object
 */
command_t* command_from_data(char* fn_argsig, void* data, int len);

/**
 * @brief Parses raw data into an existing command object
 * @param cmdo Pointer to an existing command object
 * @param fn_argsig Argument signature
 * @param len Length of data
 */
void command_from_data_inplace(command_t* cmdo, const char* fn_argsig, int len);

/* METHODS FOR COMMAND OBJECT */

/**
 * @brief Increments reference count of a command object
 * @param cmd Pointer to command object
 */
void command_hold(command_t* cmd);

/**
 * @brief Frees a command object
 * @param cmd Pointer to command object to be freed
 */
void command_free(command_t* cmd);

/**
 * @brief Allocates and initializes argument structures based on format string
 * @param fmt Format string describing argument types
 * @param rargs Pointer to allocated argument list
 * @param args Variable argument list
 * @return Boolean indicating success or failure
 */
bool command_qargs_alloc(const char* fmt, arg_t** rargs, va_list args);

/**
 * @brief Prints argument details
 * @param arg Pointer to argument to be printed
 */
void command_arg_print(arg_t* arg);

/**
 * @brief Frees an argument's internal resources
 * @param arg Pointer to argument
 */
void command_arg_inner_free(arg_t* arg);

/**
 * @brief Frees a list of arguments
 * @param arg Pointer to first argument in list
 */
void command_args_free(arg_t* arg);

/**
 * @brief Clones an argument structure
 * @param arg Pointer to argument to be cloned
 * @return Pointer to newly allocated argument
 */
arg_t* command_args_clone(arg_t* arg);

/**
 * @brief Prints command details
 * @param cmd Pointer to command to be printed
 */
void command_print(command_t* cmd);

/**
 * @brief Converts a command to a string
 * @param cmd Command to be converted
 * @return String representation of the command
 * @todo will need to be updated to include all commands
 */
const char* command_to_string(jamcommand_t cmd);
#endif