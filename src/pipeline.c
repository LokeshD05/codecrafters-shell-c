#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "builtin.h"

void pipeline_fn(char** arguments,int pipe_idx){
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
        }perror("error in pipeline.c");
        exit(1);
    }

    pid_t child2 = fork();
    if(child2 == 0){
        dup2(fd[0],STDIN_FILENO);
        close(fd[0]);
        close(fd[1]);
        if(isbuilt_in(rightcmd[0])){
            run_builtin(rightcmd);
        }
        else{
            execvp(rightcmd[0],rightcmd);
        }
        perror("error in pipeline");
        exit(1);
    }
    close(fd[0]);
    close(fd[1]);

    wait(NULL);
    wait(NULL);
}