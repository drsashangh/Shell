#include "prompt.h"
#include <stdio.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>

char home_dir[256];

void set_home_dir() 
{
    struct passwd *pw=getpwuid(geteuid());
    if(pw) 
    {
        strcpy(home_dir,pw->pw_dir);
    } 
    else 
    {
        strcpy(home_dir,"/");
    }
}

void display_prompt() 
{
    char username[256];
    char hostname[256];
    char current_path[256];

    struct passwd *pw=getpwuid(geteuid());
    if(pw) 
    {
        strcpy(username,pw->pw_name);
    } 
    else 
    {
        strcpy(username,"unknown");
    }

    if(gethostname(hostname,sizeof(hostname))!=0) 
    {
        strcpy(hostname,"unknown");
    }

    if(getcwd(current_path,sizeof(current_path))==NULL) 
    {
        perror("getcwd");
        return;
    }

    char display_path[256];
    if(strcmp(current_path,home_dir)==0) 
    {
        strcpy(display_path,"~");
    } 
    else if(strncmp(current_path,home_dir,strlen(home_dir))==0) 
    {
        snprintf(display_path,sizeof(display_path),"~%s",current_path+strlen(home_dir));
    } 
    else 
    {
        strcpy(display_path,current_path);
    }

    printf("<%s@%s:%s> ",username,hostname,display_path);
    fflush(stdout);
}