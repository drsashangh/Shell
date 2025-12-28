#include "signals.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

pid_t foreground_pid=-1;

void sigint_handler(int signum) 
{
    if(foreground_pid>0) 
    {
        kill(-foreground_pid,SIGINT);
    }
}

void sigtstp_handler(int signum) 
{
    if(foreground_pid>0) 
    {
        kill(-foreground_pid,SIGTSTP);
    }
}

void setup_signal_handlers() 
{
    struct sigaction sa_int,sa_tstp;

    sa_int.sa_handler=sigint_handler;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags=0;
    sigaction(SIGINT,&sa_int,NULL);

    sa_tstp.sa_handler=sigtstp_handler;
    sigemptyset(&sa_tstp.sa_mask);
    sa_tstp.sa_flags=SA_RESTART;
    sigaction(SIGTSTP,&sa_tstp,NULL);
}