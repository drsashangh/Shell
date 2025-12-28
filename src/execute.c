#include "execute.h"
#include "command.h"
#include "jobs.h"
#include "activities.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include "signals.h"
#include "job_control.h"
#include <errno.h>

int shell_execute_command(Command *cmd,int is_background)
{
    pid_t process_id;
    int status;
    char absolute_input_path[1024];
    extern pid_t foreground_pid;

    if(cmd->args==NULL || cmd->args[0]==NULL) 
    {
        return 0;
    }

    process_id=fork();
    if(process_id<0) 
    {
        perror("fork");
        return 1;
    } 
    else if(process_id==0) 
    {
        if(is_background) 
        {
            setpgid(0,0);
        } 
        else 
        {
            tcsetpgrp(STDIN_FILENO,getpgrp());
        }
        
        if(cmd->input_file!=NULL) 
        {
            if(realpath(cmd->input_file,absolute_input_path)==NULL) 
            {
                fprintf(stderr,"No such file or directory\n");
                _exit(1);
            }
            int fd=open(absolute_input_path,O_RDONLY);
            if(fd==-1) 
            {
                fprintf(stderr,"No such file or directory\n");
                _exit(1);
            }
            if(dup2(fd,STDIN_FILENO)==-1) 
            {
                perror("dup2 for input redirection");
                close(fd);
                _exit(1);
            }
            close(fd);
        }

        if(cmd->output_file!=NULL) 
        {
            int open_flags= O_WRONLY | O_CREAT;
            if(cmd->append_output) 
            {
                open_flags |= O_APPEND;
            } 
            else 
            {
                open_flags |= O_TRUNC;
            }

            int fd=open(cmd->output_file,open_flags,0644);
            if(fd==-1) 
            {
                perror("open for output redirection");
                _exit(1);
            }
            if(dup2(fd,STDOUT_FILENO)==-1) 
            {
                perror("dup2 for output redirection");
                close(fd);
                _exit(1);
            }
            close(fd);
        }
        
        execvp(cmd->args[0],cmd->args);
        
        if(errno==ENOENT) 
        {
            fprintf(stderr,"%s: command not found\n",cmd->args[0]);
        } 
        else 
        {
            perror(cmd->args[0]);
        }
        _exit(1);
    } 
    else 
    {
        if(is_background) 
        {
            add_job(process_id,cmd->args[0]);
            add_activities_job(process_id,cmd->args[0]);
        } 
        else 
        {
            foreground_pid=process_id;
            waitpid(process_id,&status,WUNTRACED);
            if(WIFSTOPPED(status)) 
            {
                printf("\n[%d] Stopped %s\n",process_id,cmd->args[0]);
                add_activities_job(process_id,cmd->args[0]);
            }
            foreground_pid=-1;
            tcsetpgrp(STDIN_FILENO,getpgrp());
        }
    }
    return 0;
}