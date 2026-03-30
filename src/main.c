#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX 1024

int isbuilt_in(char *cmd)
{
  return strcmp(cmd, "echo") == 0 ||
         strcmp(cmd, "exit") == 0 ||
         strcmp(cmd, "type") == 0 ||
         strcmp(cmd, "pwd") == 0;
}

void findPath(char *cmd)
{
  char *path_env = getenv("PATH");

  if (!path_env)
  {
    printf("%s: not found\n");
    return;
  }

  char path_copy[MAX];
  strncpy(path_copy, path_env, MAX - 1);
  path_copy[MAX - 1] = '\0';

  char *dir = strtok(path_copy, ":");

  while (dir)
  {
    char full_path[MAX];
    snprintf(full_path, sizeof(full_path), "%s/%s", dir, cmd);

    if (access(full_path, X_OK) == 0)
    {
      printf("%s is %s\n", cmd, full_path);
      return;
    }

    dir = strtok(NULL, ":");
  }

  printf("%s: not found\n", cmd);
}

int main(int argc, char *argv[])
{

  setbuf(stdout, NULL);
  while (1)
  {
    printf("$ ");

    char input[100];
    fgets(input, sizeof(input), stdin);

    input[strlen(input) - 1] = '\0';

    // exit command
    if (strcmp(input, "exit") == 0)
    {
      break;
    }

    // echo command
    else if (strncmp(input, "echo ", 5) == 0)
    {
      char command[MAX];
      char *arguments[MAX];

      memset(command, 0, sizeof(command));
      memset(arguments, 0, sizeof(arguments));

      bool single_quoted = false;

      int argc = 0; // act as ptr to argumenst in input
      int i = 0;
      int j = 0;

      while (input[i] != '\0')
      {
        char c = input[i];
        if (c == '\'')
        {
          single_quoted = single_quoted ? false : true;
          i++;
          continue;
        }
        else if ((c == ' ' || c == '\t') && !single_quoted)
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
          if (argc == 0 || command[j - 1] == '\0')
            arguments[argc++] = command + j;
          j++;
        }
        i++;
      }

      arguments[argc] = NULL;
      for(int i =1 ;i< argc;i++){
        if(i > 1)printf(" "); // add space between each arg
        printf("%s",arguments[i]);
      }
      printf("\n");
    }

    // type command
    else if (strncmp(input, "type ", 5) == 0)
    {
      char *type = strtok(input, " ");
      char *cmd = strtok(NULL, " ");

      if (isbuilt_in(cmd))
      {
        printf("%s is a shell builtin\n", cmd);
      }
      else
      {
        findPath(cmd);
      }
    }

    // pwd command
    else if (strncmp(input, "pwd", 3) == 0)
    {
      char cwd[MAX];
      if (getcwd(cwd, sizeof(cwd)) != NULL)
      {
        printf("%s\n", cwd);
      }
      else
        perror("getcwd error");
    }

    // cd command
    else if (strncmp(input, "cd ", 3) == 0)
    {
      char *cd = strtok(input, " ");
      char *path = strtok(NULL, ""); // the path

      if (path == NULL)
      {
        printf("cd: missing argument\n");
      }

      // ~
      if (strncmp(path, "~", 1) == 0)
      {
        char *home_dir = getenv("HOME");

        if (!home_dir)
        {
          printf("No home\n");
          continue;
        }

        char full_path[MAX];
        snprintf(full_path, sizeof(full_path), "%s%s", home_dir, path + 1);

        if (chdir(full_path) != 0)
        {
          printf("cd: %s: No such file or directory\n", full_path);
        }
      }
      else
      {
        if (chdir(path) != 0)
        {
          printf("cd: %s: No such file or directory\n", path);
        }
      }
    }

    // running external programs
    else
    {
      char *args[10];
      int i = 0;

      args[i] = strtok(input, " ");
      while (args[i] != NULL)
      {
        i++;
        args[i] = strtok(NULL, " ");
      }

      // fork
      pid_t pid = fork();

      if (pid == 0)
      {
        execvp(args[0], args);

        //* if error occured
        printf("%s: command not found\n", args[0]);
        exit(1);
      }
      else
      {
        wait(NULL);
      }
    }
  }
  return 0;
}
