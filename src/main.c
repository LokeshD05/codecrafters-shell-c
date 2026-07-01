#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "pipeline.h"
#include "builtin.h"
#include "history.h"

#define MAX 1024





int parser(char * input , char ** arguments, char *command){
  
      memset(command, 0, MAX);
      memset(arguments, 0, MAX * sizeof(char *));

      bool single_quoted = false;
      bool double_quoted = false;
      bool back_slash = false;
      int argc = 0; // act as ptr to arguments in input
      int i = 0;
      int j = 0;

      while (input[i] != '\0')
      {
        char c = input[i];
        if (c == '\'' && !double_quoted && !back_slash)
        {
          single_quoted = single_quoted ? false : true;
          i++;
          continue;
        }
        else if(c =='\"' && !single_quoted && !back_slash){
          double_quoted = double_quoted ? false : true;
          i++;
          continue;
        }
        else if(c == '\\' && !single_quoted && !back_slash){
            back_slash = true;
            i++;
            continue;
        }
        else if ((c == ' ' || c == '\t') && !single_quoted && !double_quoted && !back_slash)
        {
          if (argc == 0){      // skip leading spaces
            i++;
            continue;
          }          
          command[j++] = '\0';
        }
        else
        {
          command[j] = c;
          if(back_slash) back_slash = false;
          if (argc == 0 || command[j - 1] == '\0')
            arguments[argc++] = command + j;
          j++;
        }

        i++;
      }

      arguments[argc] = NULL;
      return argc;
}

int handle_redirection(char **arguments,int *argc,int *redirected_fd){
  for(int i = 0 ;i< *argc;i++){
    if(strcmp(arguments[i],">") == 0 || strcmp(arguments[i],"1>") == 0){
      char* filename = arguments[i+1];
      arguments[i] = NULL;
      *argc= i;
      
      *redirected_fd = STDOUT_FILENO;
      int saved_stdout = dup(STDOUT_FILENO);
      int fd = open(filename,O_WRONLY | O_CREAT | O_TRUNC,0644);
      dup2(fd,STDOUT_FILENO);
      close(fd);
      
      return saved_stdout;
    }
    else if(strcmp(arguments[i],">>") == 0 || strcmp(arguments[i],"1>>") == 0){
      char *filename = arguments[i+1];
      arguments[i] = NULL;
      *argc = i;

      *redirected_fd = STDOUT_FILENO;
      int saved_stdout = dup(STDOUT_FILENO);
      int fd = open(filename,O_WRONLY | O_CREAT | O_APPEND,0644);
      dup2(fd,STDOUT_FILENO);
      close(fd);

      return saved_stdout;
    } 
    else if(strcmp(arguments[i],"2>>") == 0){
      char *filename = arguments[i+1];
      arguments[i] = NULL;
      *argc = i;

      *redirected_fd = STDERR_FILENO;
      int saved = dup(STDERR_FILENO);
      int fd = open(filename,O_WRONLY | O_CREAT | O_APPEND,0644);
      dup2(fd,STDERR_FILENO);
      close(fd);

      return saved;
    }
    else if(strcmp(arguments[i],"2>") == 0){
      char* filename = arguments[i+1];
      arguments[i] = NULL;
      *argc= i;

      *redirected_fd = STDERR_FILENO;
      int saved = dup(STDERR_FILENO);
      int fd = open(filename,O_WRONLY | O_CREAT | O_TRUNC,0644);
      dup2(fd,STDERR_FILENO);
      close(fd);

      return saved;
    }
  }
return -1;
}

int main(int argc, char *argv[])
{
  setbuf(stdout, NULL);
  
  char command[MAX];
  char *arguments[MAX];
 
  while (1)
  {
    printf("$ ");
    
    char input[100];
    fgets(input, sizeof(input), stdin); 
    input[strlen(input) - 1] = '\0';

    add_history(input);
    
    int argc = parser(input,arguments,command);

    int pipe_idx = -1;
    for(int i = 0;i<argc;i++){
      if(strcmp(arguments[i], "|") == 0){
        pipe_idx = i;
        break;
      }
    }

    if(pipe_idx != -1){
      pipeline_fn(arguments,argc,pipe_idx);
      continue;
    }
    int redirected_fd;
    int saved_std = handle_redirection(arguments,&argc ,&redirected_fd);
    // exit command
    if (strcmp(input, "exit") == 0)
    {
      break;
    }

    if(isbuilt_in(arguments[0])){
       run_builtin(arguments);
    }
    // running external programs
    
    else
    {
      pid_t pid = fork();

      if (pid == 0)
      {
        execvp(arguments[0], arguments);

        //* if error occured
        printf("%s: command not found\n", arguments[0]);
        exit(1);
      }
      else
      {
        wait(NULL);
      }
    }
    if(saved_std !=-1){
      dup2(saved_std,redirected_fd);
      close(saved_std);
    }
  }
  return 0;
}
