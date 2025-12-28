#include "parse.h"
#include "command.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#define MAX_TOKENS 1024

char **tokenize_input(const char *input);
int atomic_parser(char **tokens,int *token_idx,Command* cmd);
int input_parser(char **tokens,int *token_idx,Command* cmd);
int output_parser(char **tokens,int *token_idx,Command* cmd);
void free_tokens(char **tokens);

char **tokenize_input(const char *input) 
{
    char **tokens=(char **)malloc(sizeof(char *)*(MAX_TOKENS+1));
    if(!tokens) return NULL;
    int token_count=0;
    const char *current=input;

    while(*current) 
    {
        while(*current && isspace((unsigned char)*current)) 
        {
            current++;
        }
        if(!*current) break;

        const char *start=current;
        if(*current=='|' || *current==';' || *current=='&') 
        {
            tokens[token_count]=(char *)malloc(2);
            if(!tokens[token_count]) 
            { 
                free_tokens(tokens); 
                return NULL; 
            }
            tokens[token_count][0]=*current;
            tokens[token_count][1]='\0';
            current++;
        } 
        else if(*current=='>') 
        {
            if(*(current+1)=='>') 
            {
                tokens[token_count]=(char *)malloc(3);
                if(!tokens[token_count]) 
                { 
                    free_tokens(tokens); 
                    return NULL; 
                }
                strcpy(tokens[token_count],">>");
                current+=2;
            } 
            else 
            {
                tokens[token_count]=(char *)malloc(2);
                if(!tokens[token_count]) 
                { 
                    free_tokens(tokens); 
                    return NULL; 
                }
                strcpy(tokens[token_count],">");
                current++;
            }
        } 
        else if(*current=='<') 
        {
            tokens[token_count]=(char *)malloc(2);
            if(!tokens[token_count]) 
            { 
                free_tokens(tokens); 
                return NULL; 
            }
            tokens[token_count][0]=*current;
            tokens[token_count][1]='\0';
            current++;
        } 
        else 
        {
            while(*current && !isspace((unsigned char)*current) &&
                   *current!='|' && *current!=';' && *current!='&' &&
                   *current!='<' && *current!='>') 
            {
                current++;
            }
            size_t len=current-start;
            tokens[token_count]=(char *)malloc(len+1);
            if(!tokens[token_count]) 
            { 
                free_tokens(tokens); 
                return NULL; 
            }
            strncpy(tokens[token_count],start,len);
            tokens[token_count][len]='\0';
        }
        token_count++;
        if(token_count>=MAX_TOKENS) 
        {
            break;
        }
    }
    tokens[token_count]=NULL;
    return tokens;
}

void free_tokens(char **tokens) 
{
    if(tokens) 
    {
        for(int i=0;tokens[i]!=NULL;i++) 
        {
            free(tokens[i]);
        }
        free(tokens);
    }
}

int atomic_parser(char **tokens,int *token_idx,Command *cmd) 
{
    if(tokens[*token_idx]==NULL ||
        strcmp(tokens[*token_idx],"|")==0 ||
        strcmp(tokens[*token_idx],";")==0 ||
        strcmp(tokens[*token_idx],"&")==0 ||
        strcmp(tokens[*token_idx],"<")==0 ||
        strcmp(tokens[*token_idx],">")==0 ||
        strcmp(tokens[*token_idx],">>")==0) 
    {
        return 0;
    }

    char **temp_args=(char**)malloc(sizeof(char*)*(MAX_TOKENS+1));
    int arg_count=0;

    while(tokens[*token_idx]!=NULL &&
           strcmp(tokens[*token_idx],"|")!=0 &&
           strcmp(tokens[*token_idx],";")!=0 &&
           strcmp(tokens[*token_idx],"&")!=0) 
    {

        if(strcmp(tokens[*token_idx],"<")==0) 
        {
            if(!input_parser(tokens,token_idx,cmd)) 
            {
                return 0;
            }
        }
        else if(strcmp(tokens[*token_idx],">")==0 || strcmp(tokens[*token_idx],">>")==0) 
        {
            if(!output_parser(tokens,token_idx,cmd)) 
            {
                return 0;
            }
        } 
        else 
        {
            temp_args[arg_count++]=strdup(tokens[*token_idx]);
            (*token_idx)++;
        }
    }
    temp_args[arg_count]=NULL;
    cmd->args=temp_args;
    return 1;
}

int input_parser(char **tokens,int *token_idx,Command *cmd) 
{
    if(!tokens[*token_idx] || strcmp(tokens[*token_idx],"<")!=0) return 0;
    (*token_idx)++;
    if(!tokens[*token_idx] || strcmp(tokens[*token_idx],"|")==0 ||
        strcmp(tokens[*token_idx],";")==0 || strcmp(tokens[*token_idx],"&")==0 ||
        strcmp(tokens[*token_idx],"<")==0 || strcmp(tokens[*token_idx],">")==0 ||
        strcmp(tokens[*token_idx],">>")==0) 
    {
        return 0;
    }
    if(cmd->input_file!=NULL) 
    {
        free(cmd->input_file);
    }
    cmd->input_file=strdup(tokens[*token_idx]);
    (*token_idx)++;
    return 1;
}

int output_parser(char **tokens, int *token_idx, Command *cmd) 
{
    if(!tokens[*token_idx] || (strcmp(tokens[*token_idx],">")!=0 && strcmp(tokens[*token_idx],">>")!=0)) return 0;

    if(cmd->output_file!=NULL) 
    {
        free(cmd->output_file);
    }
    if(strcmp(tokens[*token_idx],">>")==0) 
    {
        cmd->append_output=1;
    } 
    else 
    {
        cmd->append_output=0;
    }
    (*token_idx)++;

    if(!tokens[*token_idx] || strcmp(tokens[*token_idx],"|")==0 ||
        strcmp(tokens[*token_idx],";")==0 || strcmp(tokens[*token_idx],"&")==0 ||
        strcmp(tokens[*token_idx],"<")==0 || strcmp(tokens[*token_idx],">")==0 ||
        strcmp(tokens[*token_idx],">>")==0) 
    {
        return 0;
    }
    cmd->output_file=strdup(tokens[*token_idx]);
    (*token_idx)++;
    return 1;
}

int parse_command_line(char *input_str,Pipeline *p) 
{
    p->pipeline=NULL;
    p->command_count=0;
    p->is_background=0;
    p->is_sequential=0;

    char **tokens=tokenize_input(input_str);
    if(!tokens) return 0;

    int token_idx=0;
    Command** temp_commands=(Command**)malloc(sizeof(Command*)*MAX_TOKENS);
    int command_count=0;

    while(tokens[token_idx]!=NULL) 
    {
        Command *cmd=(Command*)malloc(sizeof(Command));
        if(cmd==NULL) 
        {
            for(int i=0;i<command_count;i++) 
            {
                if(temp_commands[i]!=NULL) 
                {
                    free(temp_commands[i]->args);
                    free(temp_commands[i]);
                }
            }
            free(temp_commands);
            return 0;
        }
        cmd->args=NULL;
        cmd->input_file=NULL;
        cmd->output_file=NULL;
        cmd->append_output=0;

        if(!atomic_parser(tokens,&token_idx,cmd)) 
        {
            return 0;
        }
        temp_commands[command_count++]=cmd;

        if(tokens[token_idx]!=NULL) 
        {
            if(strcmp(tokens[token_idx],"|")==0) 
            {
                token_idx++;
            } 
            else if(strcmp(tokens[token_idx],";")==0) 
            {
                p->is_sequential=1;
                token_idx++;
            } 
            else if(strcmp(tokens[token_idx],"&")==0) 
            {
                p->is_background=1;
                token_idx++;
            } 
            else 
            {
                return 0;
            }
        }
    }

    p->pipeline=temp_commands;
    p->command_count=command_count;

    free_tokens(tokens);
    return 1;
}