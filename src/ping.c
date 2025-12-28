#include "ping.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

void perform_ping(char **command_arguments) 
{
    if(command_arguments==NULL || command_arguments[1]==NULL || command_arguments[2]==NULL || command_arguments[3]!=NULL) 
    {
        fprintf(stderr,"Invalid Syntax for ping command.\n");
        return;
    }

    pid_t target_pid=(pid_t)atoi(command_arguments[1]);
    int signal_number=atoi(command_arguments[2]);

    int actual_signal=signal_number % 32;

    if(kill(target_pid, actual_signal)==0) 
    {
        printf("Sent signal %d to process with pid %d\n",signal_number,target_pid);
    } 
    else 
    {
        if(errno==ESRCH) 
        {
            fprintf(stderr,"No such process found\n");
        } 
        else 
        {
            perror("Error sending signal");
        }
    }
}