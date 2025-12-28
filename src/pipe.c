#include "pipe.h"
#include "execute.h"
#include "jobs.h"
#include "activities.h"
#include "hop.h"
#include "reveal.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>

void free_pipeline_memory(Pipeline *p) 
{
    if(p==NULL) 
    {
        return;
    }
    for(int i=0;i<p->command_count;i++) 
    {
        if(p->pipeline[i]!=NULL) 
        {
            if(p->pipeline[i]->args!=NULL) 
            {
                for(int j=0;p->pipeline[i]->args[j]!=NULL;j++) 
                {
                    free(p->pipeline[i]->args[j]);
                }
                free(p->pipeline[i]->args);
            }
            if(p->pipeline[i]->input_file!=NULL) 
            {
                free(p->pipeline[i]->input_file);
            }
            if(p->pipeline[i]->output_file!=NULL) 
            {
                free(p->pipeline[i]->output_file);
            }
            free(p->pipeline[i]);
        }
    }
    free(p->pipeline);
}

int execute_pipeline(Pipeline *p) 
{
    if(p==NULL || p->command_count==0) 
    {
        return 0;
    }

    if(p->command_count==1) 
    {
        Command *cmd=p->pipeline[0];
        if(cmd->args!=NULL && cmd->args[0]!=NULL) 
        {
            if(strcmp(cmd->args[0],"hop")==0)
            {
                hop_execution(cmd->args);
                return 0;
            } 
            else if(strcmp(cmd->args[0],"reveal")==0) 
            {
                execute_reveal(cmd);
                return 0;
            } 
            else 
            {
                return shell_execute_command(cmd,p->is_background);
            }
        }
        return 0;
    }

    int num_commands=p->command_count;
    int num_pipes=num_commands-1;
    int pipe_fds[num_pipes][2];
    pid_t pids[num_commands];
    int status;

    for(int i=0;i<num_pipes;i++) 
    {
        if(pipe(pipe_fds[i])==-1) 
        {
            perror("pipe");
            return 1;
        }
    }

    for(int i=0;i<num_commands;i++) 
    {
        pids[i]=fork();
        if(pids[i]<0) 
        {
            perror("fork");
            return 1;
        } 
        else if(pids[i]==0) 
        {
            if(p->is_background) 
            {
                setpgid(0,0);
            }
            
            if(i>0) 
            {
                dup2(pipe_fds[i-1][0],STDIN_FILENO);
            }
            if(i<num_pipes) 
            {
                dup2(pipe_fds[i][1],STDOUT_FILENO);
            }

            for(int j=0;j<num_pipes;j++) 
            {
                close(pipe_fds[j][0]);
                close(pipe_fds[j][1]);
            }
            
            shell_execute_command(p->pipeline[i],p->is_background);
            exit(1);
        }
    }

    for(int i=0;i<num_pipes;i++) 
    {
        close(pipe_fds[i][0]);
        close(pipe_fds[i][1]);
    }
    
    if(p->is_background) 
    {
        add_job(pids[num_commands-1],p->pipeline[num_commands-1]->args[0]);
        add_activities_job(pids[num_commands-1],p->pipeline[num_commands-1]->args[0]);
    } 
    else 
    {
        for(int i=0;i<num_commands;i++) 
        {
            waitpid(pids[i],&status,0);
        }
    }
    return 0;
}

int execute_sequential(Pipeline *p) 
{
    if(p==NULL || p->command_count==0) 
    {
        return 0;
    }
    
    for(int i=0;i<p->command_count;i++) 
    {
        Command* cmd=p->pipeline[i];
        if(cmd->args!=NULL && cmd->args[0]!=NULL) 
        {
            if(strcmp(cmd->args[0],"hop")==0) 
            {
                hop_execution(cmd->args);
            } 
            else if(strcmp(cmd->args[0],"reveal")==0) 
            {
                execute_reveal(cmd);
            } 
            else 
            {
                shell_execute_command(cmd,0);
            }
        }
    }
    return 0;
}