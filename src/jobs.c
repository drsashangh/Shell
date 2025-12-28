#include "jobs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "activities.h"

#define MAX_JOBS 20

static Job jobs[MAX_JOBS];
static int next_job_id=1;
static int job_count=0;

void init_job_management() {

    for(int i=0;i<MAX_JOBS;i++) 
    {
        jobs[i].pid=0;
        jobs[i].command_name=NULL;
        jobs[i].job_id=0;
        jobs[i].status=0;
    }
    next_job_id=1;
    job_count=0;
}

Job *find_job_by_pid(pid_t pid) 
{
    for(int i=0;i<job_count;i++) 
    {
        if(jobs[i].pid==pid) 
        {
            return &jobs[i];
        }
    }
    return NULL;
}

Job *find_job_by_id(int job_id) 
{
    for(int i=0;i<job_count;i++) 
    {
        if(jobs[i].job_id==job_id) 
        {
            return &jobs[i];
        }
    }
    return NULL;
}

Job *find_most_recent_job() 
{
    if(job_count>0) 
    {
        return &jobs[job_count-1];
    }
    return NULL;
}

void remove_job_by_pid(pid_t pid) 
{
    for(int i=0;i<job_count;i++) 
    {
        if(jobs[i].pid==pid) 
        {
            free(jobs[i].command_name);
            jobs[i].pid=0;
            for(int j=i;j<job_count-1;j++) 
            {
                jobs[j]=jobs[j+1];
            }
            jobs[job_count-1].pid=0;
            jobs[job_count-1].command_name=NULL;
            job_count--;
            break;
        }
    }
}

void check_background_jobs() 
{
    int status;
    pid_t pid;
    for(int i=0;i<job_count;i++) 
    {
        if(jobs[i].pid!=0) 
        {
            pid=waitpid(jobs[i].pid,&status,WNOHANG);
            if(pid>0) 
            {
                if(WIFEXITED(status)) 
                {
                    printf("[%d] %s with pid %d exited normally\n",jobs[i].job_id,jobs[i].command_name,jobs[i].pid);
                } 
                else if(WIFSIGNALED(status)) 
                {
                    printf("[%d] %s with pid %d exited abnormally\n",jobs[i].job_id,jobs[i].command_name,jobs[i].pid);
                }
                remove_job_by_pid(jobs[i].pid);
                remove_activities_job(jobs[i].pid);
                i--; 
            }
            else if(pid==0) 
            {
                
            }
        }
    }
}

void add_job(pid_t pid,const char *command_name) 
{
    if(job_count>=MAX_JOBS) 
    {
        fprintf(stderr,"Job list is full,cannot add new job.\n");
        return;
    }
    jobs[job_count].pid=pid;
    jobs[job_count].job_id=next_job_id++;
    jobs[job_count].command_name=strdup(command_name);
    jobs[job_count].status=0; 
    printf("[%d] %d\n",jobs[job_count].job_id,pid);
    job_count++;
}

void cleanup_jobs() 
{
    for(int i=0;i<job_count;i++)
    {
        if(jobs[i].pid!=0) 
        {
            kill(jobs[i].pid,SIGKILL);
            waitpid(jobs[i].pid,NULL,0); 
            free(jobs[i].command_name);
        }
    }
}