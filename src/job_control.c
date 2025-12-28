#include "job_control.h"
#include "jobs.h"
#include "activities.h"
#include "signals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

void execute_fg(char **args) 
{
    Job *job;
    if(args[1]==NULL) 
    {
        job=find_most_recent_job();
    } 
    else 
    {
        int job_id=atoi(args[1]);
        job=find_job_by_id(job_id);
    }

    if(job==NULL || job->pid==0) 
    {
        fprintf(stderr,"No such job\n");
        return;
    }

    printf("%s\n",job->command_name);
    
    if(job->status==1) 
    { 
        kill(job->pid,SIGCONT);
        job->status=0;
    }

    tcsetpgrp(STDIN_FILENO,job->pid);
    foreground_pid=job->pid;
    
    int status;
    waitpid(job->pid,&status,WUNTRACED);

    tcsetpgrp(STDIN_FILENO,getpgrp());
    foreground_pid=-1;

    if(WIFEXITED(status) || WIFSIGNALED(status)) 
    {
        remove_job_by_pid(job->pid);
        remove_activities_job(job->pid);
    }
    else if(WIFSTOPPED(status)) 
    {
        job->status=1;
        printf("\n[%d] Stopped %s\n",job->job_id,job->command_name);
    }
}

void execute_bg(char **args) 
{
    Job *job;
    if(args[1]==NULL) 
    {
        job=find_most_recent_job();
    }
    else 
    {
        int job_id=atoi(args[1]);
        job=find_job_by_id(job_id);
    }

    if(job==NULL || job->pid==0) 
    {
        fprintf(stderr,"No such job\n");
        return;
    }

    if(job->status==0) 
    {
        printf("Job already running\n");
        return;
    }
    
    kill(job->pid,SIGCONT);
    job->status=0;
    printf("[%d] %s &\n",job->job_id,job->command_name);
}