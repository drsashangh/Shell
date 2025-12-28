#include "activities.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_ACTIVITIES 50

static ActivitiesJob activities_list[MAX_ACTIVITIES];
static int activities_count=0;

void add_activities_job(pid_t pid,const char *command_name) 
{
    if(activities_count>=MAX_ACTIVITIES) 
    {
        fprintf(stderr,"Activities list is full,cannot add new job.\n");
        return;
    }
    activities_list[activities_count].pid=pid;
    activities_list[activities_count].command_name=strdup(command_name);
    activities_list[activities_count].state=strdup("Running");
    activities_count++;
}

void remove_activities_job(pid_t pid) 
{
    for(int i=0;i<activities_count;i++) 
    {
        if(activities_list[i].pid==pid) 
        {
            free(activities_list[i].command_name);
            free(activities_list[i].state);
            activities_list[i].pid=0;
            for(int j=i;j<activities_count-1;j++) 
            {
                activities_list[j]=activities_list[j+1];
            }
            activities_list[activities_count-1].pid=0;
            activities_list[activities_count-1].command_name=NULL;
            activities_list[activities_count-1].state=NULL;
            activities_count--;
            break;
        }
    }
}

void check_activities_jobs() 
{
    int status;
    pid_t pid;
    for(int i=0;i<activities_count;i++) 
    {
        if(activities_list[i].pid!=0) 
        {
            pid=waitpid(activities_list[i].pid,&status,WNOHANG | WUNTRACED);
            if(pid>0) 
            {
                if(WIFEXITED(status) || WIFSIGNALED(status)) 
                {
                    remove_activities_job(activities_list[i].pid);
                    i--; 
                } 
                else if(WIFSTOPPED(status)) 
                {
                    free(activities_list[i].state);
                    activities_list[i].state=strdup("Stopped");
                } 
                else if(WIFCONTINUED(status)) 
                {
                    free(activities_list[i].state);
                    activities_list[i].state=strdup("Running");
                }
            }
        }
    }
}

static int compare_activities_jobs(const void *a,const void *b) 
{
    const ActivitiesJob *job_a=(const ActivitiesJob *)a;
    const ActivitiesJob *job_b=(const ActivitiesJob *)b;
    return strcmp(job_a->command_name,job_b->command_name);
}

void execute_activities() 
{
    qsort(activities_list,activities_count,sizeof(ActivitiesJob),compare_activities_jobs);
    for(int i=0;i<activities_count;i++) 
    {
        if(activities_list[i].pid!=0) 
        {
            printf("[%d] : %s - %s\n", activities_list[i].pid,activities_list[i].command_name,activities_list[i].state);
        }
    }
}

void cleanup_activities() 
{
    for(int i=0;i<activities_count;i++) 
    {
        if(activities_list[i].pid!=0) 
        {
            free(activities_list[i].command_name);
            free(activities_list[i].state);
        }
    }
}