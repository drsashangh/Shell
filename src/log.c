#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

static char* command_history[MAX_HISTORY_SIZE];
static int history_count=0;


static char* get_history_file_path() 
{
    struct passwd *pw=getpwuid(getuid());
    if(pw==NULL) 
    {
        return NULL;
    }
    
    char* home_dir=pw->pw_dir;
    char* file_path=(char*)malloc(strlen(home_dir)+strlen(HISTORY_FILE_NAME)+2);
    if(file_path==NULL) 
    {
        return NULL;
    }
    sprintf(file_path, "%s/%s",home_dir,HISTORY_FILE_NAME);
    return file_path;
}

void init_command_history() 
{
    for(int i=0;i<MAX_HISTORY_SIZE;i++) 
    {
        command_history[i]=NULL;
    }

    char* file_path=get_history_file_path();
    if(file_path==NULL) 
    {
        return;
    }

    FILE* file=fopen(file_path,"r");
    if(file==NULL) 
    {
        free(file_path);
        return;
    }

    char line[1024];
    while(history_count<MAX_HISTORY_SIZE && fgets(line,sizeof(line),file)!=NULL) 
    {
        line[strcspn(line,"\n")]=0;
        command_history[history_count]=strdup(line);
        history_count++;
    }

    fclose(file);
    free(file_path);
}

void save_command_history() 
{
    char* file_path=get_history_file_path();
    if(file_path==NULL) 
    {
        return;
    }

    FILE* file=fopen(file_path,"w");
    if(file==NULL) 
    {
        perror("fopen");
        free(file_path);
        return;
    }

    for(int i=0;i<history_count;i++) 
    {
        if(command_history[i]!=NULL) 
        {
            fprintf(file,"%s\n",command_history[i]);
            free(command_history[i]);
            command_history[i]=NULL;
        }
    }

    fclose(file);
    free(file_path);
}

int add_to_command_history(const char* command) 
{
    if(history_count>0 && strcmp(command_history[history_count-1],command)==0) 
    {
        return 0;
    }

    if(strncmp(command,"log",3)==0) 
    {
        return 0;
    }
    
    char* new_command=strdup(command);
    if(new_command==NULL) 
    {
        return 0;
    }

    if(history_count<MAX_HISTORY_SIZE) 
    {
        command_history[history_count]=new_command;
        history_count++;
    } 
    else 
    {
        free(command_history[0]);
        for(int i=0;i<MAX_HISTORY_SIZE-1;i++) 
        {
            command_history[i]=command_history[i+1];
        }
        command_history[MAX_HISTORY_SIZE-1]=new_command;
    }
    return 1;
}

void execute_shell_log(char *const args[]) 
{
    if(args[1]==NULL) 
    { 
        for(int i=0;i<history_count;i++) 
        {
            printf("%s\n",command_history[i]);
        }
    } 
    else if(strcmp(args[1],"purge")==0) 
    {
        for(int i=0;i<history_count;i++) 
        {
            if(command_history[i]!=NULL) 
            {
                free(command_history[i]);
                command_history[i]=NULL;
            }
        }
        history_count=0;
    } 
    else if(strcmp(args[1],"execute")==0) 
    {
        if(args[2]==NULL) 
        {
            printf("log: execute needs an index.\n");
            return;
        }
    } 
    else 
    {
        printf("log: Invalid syntax.\n");
    }
}

char* get_command_from_history(int index) 
{
    if(index<=0 || index>history_count) 
    {
        return NULL;
    }
    int history_index=history_count-index;
    if(history_index>=0 && command_history[history_index]!=NULL) 
    {
        return strdup(command_history[history_index]);
    }
    return NULL;
}