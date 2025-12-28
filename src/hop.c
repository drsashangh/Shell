#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <pwd.h>

static char prev_working_directory[256]="";

void hop_execution(char **command_args) 
{
    char current_dir[256];
    char* target_dir=NULL;

    if(getcwd(current_dir,sizeof(current_dir))==NULL) 
    {
        perror("getcwd failed");
        return;
    }

    if(command_args[0]==NULL) 
    {
        struct passwd *pw=getpwuid(geteuid());
        if(pw && pw->pw_dir) 
        {
            target_dir=pw->pw_dir;
        } 
        else 
        {
            target_dir="/";
        }
        
        if(chdir(target_dir)==0) 
        {
            strncpy(prev_working_directory,current_dir,sizeof(prev_working_directory)-1);
            prev_working_directory[sizeof(prev_working_directory)-1]='\0';
        } 
        else 
        {
            fprintf(stderr,"No such directory!\n");
        }
        return;
    }

    for(int i=0;command_args[i]!=NULL;i++) 
    {
        char* arg=command_args[i];
        char temp_current_dir[256];

        if(getcwd(temp_current_dir,sizeof(temp_current_dir))==NULL) 
        {
            perror("getcwd failed");
            return;
        }

        if(strcmp(arg,"~")==0) 
        {
            struct passwd *pw=getpwuid(geteuid());
            if(pw && pw->pw_dir) 
            {
                target_dir=pw->pw_dir;
            } 
            else 
            {
                target_dir="/";
            }
        }
        else if(strcmp(arg,"-")==0) 
        {
            if(strlen(prev_working_directory)>0) 
            {
                target_dir=prev_working_directory;
            } 
            else 
            {
                fprintf(stderr,"No such directory!\n");
                return;
            }
        } 
        else 
        {
            target_dir=arg;
        }

        if(target_dir!=NULL) 
        {
            if(chdir(target_dir)==0) 
            {
                strncpy(prev_working_directory,temp_current_dir,sizeof(prev_working_directory)-1);
                prev_working_directory[sizeof(prev_working_directory)-1]='\0';
            } 
            else 
            {
                fprintf(stderr,"No such directory!\n");
            }
        }
    }
}

char *get_prev_working_directory(void) 
{
    if(strlen(prev_working_directory)==0) return NULL;
    return prev_working_directory;
}