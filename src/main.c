#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <path.h>
#define MAX 1024

int isbuilt_in(char *cmd)
{
  return strcmp(cmd, "echo") == 0 ||
         strcmp(cmd, "exit") == 0 ||
         strcmp(cmd, "type") == 0;
}

void findPath(char *cmd)
{
  char* path_env = getenv("PATH");

  if(!path_env){
    printf("%s: not found\n");
    return;
  }

  char path_copy[MAX];
  strncpy(path_copy,path_env,MAX -1);
  path_copy[MAX -1]="\0";

  char* dir = strtok(path_copy,":");

  while(dir){
    char full_path[MAX];
    snprintf(full_path,sizeof(full_path),"%s/%s",dir,cmd);

    if(access(full_path,X_OK) == 0){
      printf("%s is %s\n",cmd,full_path);
      return;
    }
    
    dir = strtok(NULL,":");
  }

  printf("%s: not found\n",cmd);
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

    if (strcmp(input, "exit") == 0)
    {
      break;
    }
    else if (strncmp(input, "echo ", 5) == 0)
    {
      printf("%s\n", input + 5);
    }

    else if (strncmp(input, "type ", 5) == 0)
    {
      char *cmd = input + 5;

      if (isbuilt_in(cmd))
      {
        printf("%s is a shell builtin\n", cmd);
      }
      else
      {
        findPath(cmd);
       
      }
    }

    else
    {
      printf("%s: command not found\n", input);
    }
  }
  return 0;
}
