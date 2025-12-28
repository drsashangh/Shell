#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#include "prompt.h"
#include "parse.h"
#include "hop.h"
#include "reveal.h"
#include "execute.h"
#include "log.h"
#include "command.h"
#include "pipe.h"
#include "jobs.h"
#include "activities.h"
#include "ping.h"
#include "signals.h"
#include "job_control.h"

#define MAX_INPUT_SIZE 2048

int main()
{
    set_home_dir();
    init_command_history();
    init_job_management();
    setup_signal_handlers(); 
    
    char input_line[MAX_INPUT_SIZE];

    while(1)
    {
        check_background_jobs();
        check_activities_jobs(); 
        display_prompt();
        
        if(fgets(input_line,sizeof(input_line),stdin)==NULL)
        {
            printf("\n");
            printf("logout\n");
            save_command_history();
            cleanup_jobs();
            cleanup_activities();
            exit(0);
        }

        input_line[strcspn(input_line,"\n")]=0;

        int is_empty=1;
        for(int i=0;input_line[i]!='\0';i++)
        {
            if(!isspace((unsigned char)input_line[i]))
            {
                is_empty=0;
                break;
            }
        }
        if(is_empty)
        {
            continue;
        }
        
        char *input_copy=strdup(input_line);
        if(input_copy==NULL) 
        {
            continue;
        }

        char *first_token=strtok(input_copy," \t\r\n");

        if(first_token!=NULL && strcmp(first_token,"log")==0)
        {
            char *args[256];
            int arg_count=0;
            char *token=strtok(input_line," \t\r\n");
            while(token!=NULL) 
            {
                args[arg_count++]=token;
                token=strtok(NULL," \t\r\n");
            }
            args[arg_count]=NULL;
            
            if(arg_count>1 && strcmp(args[1],"execute")==0)
            {
                if(arg_count<3)
                {
                    printf("log: execute requires an index.\n");
                }
                else
                {
                    int index=atoi(args[2]);
                    char *command_to_execute=get_command_from_history(index);
                    if(command_to_execute==NULL)
                    {
                        printf("log: Invalid index.\n");
                    } 
                    else 
                    {
                        add_to_command_history(command_to_execute);
                        Pipeline p;
                        if(parse_command_line(command_to_execute,&p)) 
                        {
                            if(p.pipeline[0]->args!=NULL && strcmp(p.pipeline[0]->args[0],"hop")==0) 
                            {
                                hop_execution(p.pipeline[0]->args);
                            } 
                            else if(p.pipeline[0]->args!=NULL && strcmp(p.pipeline[0]->args[0],"reveal")==0)
                            {
                                execute_reveal(p.pipeline[0]);
                            } 
                            else 
                            {
                                execute_pipeline(&p);
                            }
                        }
                        free_pipeline_memory(&p);
                        free(command_to_execute);
                    }
                }
            }
            else
            {
                add_to_command_history(input_line);
                execute_shell_log(args);
            }
            free(input_copy);
            continue;
        }
        else if(first_token!=NULL && strcmp(first_token,"activities")==0)
        {
            add_to_command_history(input_line);
            execute_activities();
            free(input_copy);
            continue;
        }
        else if(first_token!=NULL && strcmp(first_token,"ping")==0)
        {
            add_to_command_history(input_line);
            char *args[256];
            int arg_count=0;
            char *token=strtok(input_line," \t\r\n");
            while(token!=NULL) 
            {
                args[arg_count++]=token;
                token=strtok(NULL," \t\r\n");
            }
            args[arg_count]=NULL;
            perform_ping(args);
            free(input_copy);
            continue;
        }
        else if(first_token!=NULL && strcmp(first_token,"fg")==0)
        {
            add_to_command_history(input_line);
            char *args[256];
            int arg_count=0;
            char *token=strtok(input_line," \t\r\n");
            while(token!=NULL) 
            {
                args[arg_count++]=token;
                token=strtok(NULL," \t\r\n");
            }
            args[arg_count]=NULL;
            execute_fg(args);
            free(input_copy);
            continue;
        }
        else if(first_token!=NULL && strcmp(first_token,"bg")==0)
        {
            add_to_command_history(input_line);
            char *args[256];
            int arg_count=0;
            char *token=strtok(input_line," \t\r\n");
            while(token!=NULL) 
            {
                args[arg_count++]=token;
                token=strtok(NULL," \t\r\n");
            }
            args[arg_count]=NULL;
            execute_bg(args);
            free(input_copy);
            continue;
        }

        add_to_command_history(input_line);
        
        Pipeline p;
        if(parse_command_line(input_line,&p)) 
        {
            if(p.is_sequential) 
            {
                execute_sequential(&p);
            } 
            else 
            {
                execute_pipeline(&p);
            }
            free_pipeline_memory(&p);
        } 
        else 
        {
            printf("Invalid Syntax!\n");
        }
        
        free(input_copy);
    }
    
    cleanup_jobs();
    cleanup_activities();
    return 0;
}