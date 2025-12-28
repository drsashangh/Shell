#ifndef ACTIVITIES_H
#define ACTIVITIES_H

#include <sys/types.h>

typedef struct 
{
    pid_t pid;
    char *command_name;
    char *state;
}ActivitiesJob;

void add_activities_job(pid_t pid,const char *command_name);
void remove_activities_job(pid_t pid);
void check_activities_jobs();
void execute_activities();
void cleanup_activities();

#endif