#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "builtin.h"

void pipeline_fn(char **arguments, int argc, int pipe_idx)
{
    char **cmds[argc];
    int cmdCount = 0;
    int cmdStart = 0;

    for (int i = 0; i < argc; i++)
    {
        if (strcmp(arguments[i], "|") == 0)
        {
            cmds[cmdCount++] = &arguments[cmdStart];
            arguments[i] = NULL;
            cmdStart = i + 1;
        }
    }
    cmds[cmdCount++] = &arguments[cmdStart];

    int fd[cmdCount - 1][2];

    // create pipes
    for (int i = 0; i < cmdCount - 1; i++)
    {
        if (pipe(fd[i]) == -1)
        {
            perror("pipe creation error");
            exit(1);
        }
    }

    pid_t pids[cmdCount];

    for (int i = 0; i < cmdCount; i++)
    {
        pids[i] = fork();

        if (i == 0)
        {
            dup2(fd[0][1], STDOUT_FILENO);
        }
        else if (i == cmdCount - 1)
        {
            dup2(fd[i - 1][0], STDIN_FILENO);
        }
        // Middle commands
        else
        {
            dup2(fd[i - 1][0], STDIN_FILENO);
            dup2(fd[i][1], STDOUT_FILENO);
        }

        // close all pipes
        for (int j = 0; j < cmdCount - 1; j++)
        {
            close(fd[j][0]);
            close(fd[j][1]);
        }

        if (isbuilt_in(cmds[i][0]))
        {
            run_builtin(cmds[i]);
            exit(0);
        }

        execvp(cmds[i][0], cmds[i]);
        perror(cmds[i][0]);
        exit(1);
    }
    // parent closes pipes
    for (int j = 0; j < cmdCount - 1; j++)
    {
        close(fd[j][0]);
        close(fd[j][1]);
    }

    for (int i = 0; i < cmdCount; i++)
    {
        waitpid(pids[i], NULL, 0);
    }
    /*
    // code for single pipe operator
    arguments[pipe_idx] = NULL;
    char **leftcmd = arguments;
    char **rightcmd = &arguments[pipe_idx + 1];

    // pipe creation
    int fd[2];
    pipe(fd);

    pid_t child1 = fork();
    if(child1 == 0){
        dup2(fd[1],STDOUT_FILENO);
        close(fd[0]);
        close(fd[1]);

        if(isbuilt_in(leftcmd[0])){
            run_builtin(leftcmd);
            }
            else{
                execvp(leftcmd[0],leftcmd);
                perror("error in pipeline.c");
                }
                exit(1);
                }

                pid_t child2 = fork();
                if(child2 == 0){
                    dup2(fd[0],STDIN_FILENO);
                    close(fd[0]);
                    close(fd[1]);

                    if(isbuilt_in(rightcmd[0])){
                        run_builtin(rightcmd);
                        exit(0);
                        }
                        else{
                            execvp(rightcmd[0],rightcmd);
                            perror("error in pipeline");
     }
     exit(1);
     }
     close(fd[0]);
     close(fd[1]);

     wait(NULL);
     wait(NULL);
     */

}