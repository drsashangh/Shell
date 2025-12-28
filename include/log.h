#ifndef LOG_H
#define LOG_H

#define MAX_HISTORY_SIZE 15

#define HISTORY_FILE_NAME ".shell_history"

void init_command_history();

void save_command_history();

int add_to_command_history(const char *command);

void execute_shell_log(char *const args[]);

char* get_command_from_history(int index);

#endif