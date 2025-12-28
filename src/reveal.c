#include "reveal.h"
#include "command.h"
#include "hop.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <pwd.h>
#include <fcntl.h>

#define MAX_ENTRIES 1024
#define MAX_PATH_LEN 512

int compare_strings(const void *a,const void *b) 
{
    return strcmp(*(const char**)a,*(const char**)b);
}

char *resolve_reveal_path(char *arg) 
{
    static char resolved_path[MAX_PATH_LEN];
    memset(resolved_path,0,sizeof(resolved_path));

    if(!arg || strcmp(arg,".")==0) 
    {
        if(!getcwd(resolved_path,sizeof(resolved_path))) return NULL;
    }
    else if(strcmp(arg,"~")==0) 
    {
        struct passwd *pw=getpwuid(geteuid());
        if(pw) strcpy(resolved_path,pw->pw_dir);
        else strcpy(resolved_path,"/");
    }
    else if(strcmp(arg,"..")==0) 
    {
        char cwd[MAX_PATH_LEN];
        if(!getcwd(cwd,sizeof(cwd))) return NULL;
        char* last_slash=strrchr(cwd,'/');
        if(last_slash && last_slash!=cwd)
            *last_slash='\0';
        else
            strcpy(cwd,"/");
        strcpy(resolved_path,cwd);
    }
    else if(strcmp(arg,"-")==0) 
    {
        char* prev=get_prev_working_directory();
        if(!prev) return NULL;
        strcpy(resolved_path,prev);
    }
    else 
    {
        strcpy(resolved_path,arg);
    }
    return resolved_path;
}

void execute_reveal(Command *cmd) 
{
    bool show_hidden=false;
    bool line_by_line=false;
    char* target_path=NULL;
    int stdout_backup=-1;
    int fd_out=-1;

    for(int i=0;cmd->args[i]!=NULL;i++)
    {
        if(strcmp(cmd->args[i],">")==0 || strcmp(cmd->args[i],">>")==0)
        {
            if(cmd->args[i+1]) 
            {
                cmd->output_file=cmd->args[i+1];
                cmd->append_output=(strcmp(cmd->args[i],">>")==0);
                cmd->args[i]=NULL;
                break;
            }
        }
    }

    if(cmd->output_file!=NULL) 
    {
        stdout_backup=dup(STDOUT_FILENO);
        int open_flags=O_WRONLY | O_CREAT;
        if(cmd->append_output)
            open_flags |= O_APPEND;
        else
            open_flags |= O_TRUNC;

        fd_out=open(cmd->output_file,open_flags,0644);
        if(fd_out<0) 
        {
            perror("open for output redirection");
            if(stdout_backup!=-1) 
            {
                dup2(stdout_backup,STDOUT_FILENO);
                close(stdout_backup);
            }
            return;
        }

        dup2(fd_out,STDOUT_FILENO);
        close(fd_out);
    }

    for(int i=1;cmd->args[i]!=NULL;i++) 
    {
        if(cmd->args[i][0]=='-') 
        {
            for(int j=1;cmd->args[i][j]!='\0';j++) 
            {
                if(cmd->args[i][j]=='a') show_hidden=true;
                if(cmd->args[i][j]=='l') line_by_line=true;
            }
        }
        else
        {
            target_path=cmd->args[i];
            break;
        }
    }

    if(target_path==NULL)
        target_path=".";

    char* final_path=resolve_reveal_path(target_path);
    if(final_path==NULL) 
    {
        fprintf(stderr,"No such directory!\n");
        goto cleanup;
    }

    DIR* dir_stream=opendir(final_path);
    if(dir_stream==NULL) 
    {
        fprintf(stderr,"No such directory!\n");
        goto cleanup;
    }

    struct dirent* entry;
    char* entries[MAX_ENTRIES];
    int count=0;

    while((entry=readdir(dir_stream))!=NULL && count<MAX_ENTRIES)
    {
        if(!show_hidden && entry->d_name[0]=='.')
            continue;

        entries[count]=strdup(entry->d_name);
        if(entries[count]==NULL)
            break;

        count++;
    }

    qsort(entries,count,sizeof(char*),compare_strings);

    for(int i=0;i<count;i++) 
    {
        printf("%s",entries[i]);
        if(line_by_line)
            printf("\n");
        else
            printf(" ");

        fflush(stdout);  

        free(entries[i]);
    }

    if(!line_by_line)
        printf("\n");

    closedir(dir_stream);

cleanup:
    if(stdout_backup!=-1) 
    {
        fflush(stdout);
        dup2(stdout_backup,STDOUT_FILENO);
        close(stdout_backup);
    }
}
