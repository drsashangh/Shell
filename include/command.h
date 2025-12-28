#ifndef COMMAND_H
#define COMMAND_H

typedef struct 
{
    char **args;
    char *input_file;
    char *output_file;
    int append_output;
    int is_background;
}Command;

typedef struct 
{
    Command **pipeline;
    int command_count;
    int is_background;
    int is_sequential;
}Pipeline;

#endif
