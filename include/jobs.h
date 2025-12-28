#ifndef JOBS_H
#define JOBS_H

#include <sys/types.h>

typedef struct 
{
    pid_t pid;
    char *command_name;
    int job_id;
    int status;
}Job;

void init_job_management();
void add_job(pid_t pid,const char *command_name);
void remove_job_by_pid(pid_t pid);
Job *find_job_by_id(int job_id);
Job *find_most_recent_job();
void check_background_jobs();
void cleanup_jobs();

#endif