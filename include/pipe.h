#ifndef PIPE_H
#define PIPE_H

#include "command.h"

int execute_pipeline(Pipeline *p);
void free_pipeline_memory(Pipeline *p);
int execute_sequential(Pipeline *p);

#endif