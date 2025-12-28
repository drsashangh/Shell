#ifndef SIGNALS_H
#define SIGNALS_H

#include <sys/types.h>

extern pid_t foreground_pid;
void sigint_handler(int signum);
void sigtstp_handler(int signum);
void setup_signal_handlers();

#endif