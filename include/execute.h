#ifndef EXECUTE_H
#define EXECUTE_H

#include "command.h"

int shell_execute_command(Command *cmd,int is_background);

#endif