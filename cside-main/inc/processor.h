// #pragma once
#include "tboard.h"
#include "command.h"
#include "utils.h"

bool process_message(tboard_t* tboard, command_t* cmd);

void processor_init();