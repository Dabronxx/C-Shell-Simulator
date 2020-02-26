#define _BSD_SOURCE
#include "command_line.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "errno.h"
#include "signal.h"
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include "history_queue.h"
void eval(struct CommandLine * command);
pid_t pid;

void sigHandler(int sig)
{
    
}

//Function executes cd command in shell
int cd(struct CommandLine * command)
{
    return chdir(command->arguments[1]);
}
//Function takes care of the built in commands
bool builtin_command(struct CommandLine * command)
{
   
    if (!strcmp(command->arguments[0], "quit") || !strcmp(command->arguments[0], "exit"))
    {
        freeHistory();
        exit(0);
    }/* quit command */
    
    
    if (!strcmp(command->arguments[0], "cd"))
    {
        if (cd(command) == -1)
        {
            printf("%s No such file or directory\n", command->arguments[1]);
        }
        
        return true;
    }

    if (!strcmp(command->arguments[0], "history"))
    {
        //add_to_history(command);
        if(command->arguments[1] == NULL)
        {
            print_history(10);
        }
        else
        {
            print_history(atoi(command->arguments[1]));
        }
        return true;
    }
    
    return false; /* Not a builtin command */
}

//Function creates redirect output and input files
//Branko
bool redirection(struct CommandLine * command)
{
    bool redirect = true;
    
    int outputFile;
    
    int inputFile;
    
    for (int i = 0; i < command->argCount; i++)
    {
        if (!strcmp(command->arguments[i], "1>") || !strcmp(command->arguments[i], "2>"))
        {
            if (command->arguments[i + 1] == NULL)
            {
                printf("No accompanying output file\n");
                
                redirect = false;
            }
            else
            {
                outputFile = open(command->arguments[i+1],  O_WRONLY | O_CREAT, 0666);
                
                dup2(outputFile, command->arguments[i][0] - '0');
                
                close(outputFile);
                
                command->arguments[i] = NULL;
            }
            
            
        }
        else if(!strcmp(command->arguments[i], "<"))
        {
            if (command->arguments[i + 1] == NULL)
            {
                printf("No accompanying input file\n");
                
                redirect = false;
            }
            else if((inputFile = open(command->arguments[i+1], O_RDONLY)) < 0)
            {
                printf("Input file does not exist\n");
                
                redirect = false;
            }
            else
            {
                dup2(inputFile, 0);
                
                close(inputFile);
                
                command->arguments[i] = NULL;
            }
        }
    }
    return redirect;
}
//Function returns full path of the executable command
//Branko
char * fullPath(struct CommandLine * command)
{
    char * shell_path = getenv ("PATH");
    
    if(access(command->arguments[0], F_OK && X_OK) == 0)
    {
        return command->arguments[0];
    }
    else
    {
        char * full_path = malloc(MAX_LINE_LENGTH*sizeof(char));
    
        char * copy = malloc(MAX_LINE_LENGTH*sizeof(char));
        
        char * token = malloc(MAX_LINE_LENGTH*sizeof(char));
        
        copy = strndup(shell_path, MAX_LINE_LENGTH);
        
        token = strtok(copy, ":");
        
        while (token != NULL)
        {
            sprintf(full_path, "%s/%s", token, command->arguments[0]);
            
            if(access(full_path, F_OK && X_OK) == 0)
            {
                return full_path;
            }
            token = strtok(NULL, ":");
        }
    }
    return NULL;
}
//Function executes pipe commands
//Yiwen
bool pip(struct CommandLine * command)
{
    int status;
    
    for (int i = 0; i < command->argCount; i++)
    {
        if (!strcmp(command->arguments[i], "|"))
        {
            int isParent;
            
            int apipe[2];
            
            pipe(apipe);
            
            if((isParent = fork()) == 0)
            {
                close(apipe[0]);
                
                dup2(apipe[1],1);
                
                struct CommandLine firstCommand;
                
                initializeCommand(&firstCommand);
                
                firstCommand.argCount = 2;
                
                firstCommand.arguments[0] = malloc(strlen(command->arguments[i-2]));
                
                firstCommand.arguments[1] = malloc(strlen(command->arguments[i-1]));
                
                strcpy(firstCommand.arguments[0], command->arguments[i-2]);
                
                strcpy(firstCommand.arguments[1], command->arguments[i-1]);
                
                char *firstPath = fullPath(&firstCommand);
                
                if(firstPath == NULL)
                {
                    printf("%s Command Not Found.\n", firstCommand.arguments[0]);
                    
                    return true;
                }
                execv(firstPath, firstCommand.arguments);
                
                close(apipe[1]);
                
                freeCommand(&firstCommand);
                
                perror("child exec failed\n");
                
            }
            else
            {
                waitpid(isParent, &status, 0);
                
                isParent = fork();
                
                if (isParent == 0)
                {
                    close(apipe[1]);
                    
                    dup2(apipe[0],0);
                    
                    struct CommandLine secondCommand;
                    
                    initializeCommand(&secondCommand);
                    
                    secondCommand.argCount = 2;
                    
                    secondCommand.arguments[0] = malloc(strlen(command->arguments[i+1]));
                    
                    secondCommand.arguments[1] = malloc(strlen(command->arguments[i+2]));
                    
                    strcpy(secondCommand.arguments[0], command->arguments[i+1]);
                    
                    strcpy(secondCommand.arguments[1], command->arguments[i+2]);
                    
                    char *secondPath = fullPath(&secondCommand);
                    
                    if(secondPath == NULL)
                    {
                        printf("%s Command Not Found.\n", secondCommand.arguments[0]);
                        return true;
                    }
                    
                    execv(secondPath, secondCommand.arguments);
                    
                    
                    close(apipe[0]);
                   freeCommand(&secondCommand);
                    perror("parent exec failed\n");
                    
                }
                return 1;
            }
            
        }
    }
    
    return 0;
}
//Function executes command
//Yiwen
void eval(struct CommandLine * command)
{
    char * path = fullPath(command);
    
    if (command->arguments[0] == NULL)
    {
        return; /* Ignore empty lines */
    }
    
    if (!builtin_command(command) && !pip(command))
    {
        int status;
        
        if ((pid = fork()) == 0)
        { /* Child runs user job */
            if (path == NULL )
            {
                printf("%s: Command not found.\n", command->arguments[0]);
                
                return;
            }
            if (!redirection(command))
            {
                return;
            }
            execv(path, command->arguments);
        }
        else
        {
            if (!command->background)
            {
                waitpid(pid, &status, 0);
            }
            
        }
        
    }
    
    return;
}
//Function evaluates commands that have exclamation marks
//Yiwen
bool isExclamation(struct CommandLine * command)
{
    if (command->arguments[0][0] == '!')
    {
        if(strlen(command->arguments[0]) == 1)
        {
            printf("%s: Command not found.\n", command->arguments[0]);
            
            add_to_history(command);
            
            return false;
        }
        if (command->arguments[0][1] == '!')
        {
            if(get_last_command() == NULL)
            {
                printf("Command not in history\n");
                
                add_to_history(command);
                
                return false;
            }
            copyCommand(command, get_last_command());
        }
        else
        {
            int command_num = command->arguments[0][1] - '0';
            
            if (get_command(command_num) == NULL)
            {
                printf("Command not in history\n");
                
                add_to_history(command);
                
                return false;
            }
            copyCommand(command, get_command(command_num));
        }
        
    }
    return true;
    
}
int main(int argc, const char **argv)
{
    signal(SIGINT, sigHandler);
    
    char cmdline[MAX_LINE_LENGTH];
    
    struct CommandLine command;
    
    while (1)
    {
        printf("tosh$ ");
        
        fgets(cmdline, MAX_LINE_LENGTH, stdin);
        
        if (feof(stdin))
        {
            exit(0);
        }
        bool gotLine = parseLine(&command, cmdline);
        
        if (gotLine)
        {
            if(!isExclamation(&command))
            {
                continue;
            }
            add_to_history(&command);
        
            eval(&command);
            
            freeCommand(&command);
        }
        
    }
    
}




