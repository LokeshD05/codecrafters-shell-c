#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX 1024

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

int isbuilt_in(char *cmd)
{
    if (cmd == NULL)
        return 0;

    return strcmp(cmd, "history") == 0 ||
           strcmp(cmd, "echo") == 0 ||
           strcmp(cmd, "exit") == 0 ||
           strcmp(cmd, "type") == 0 ||
           strcmp(cmd, "pwd") == 0 ||
           strcmp(cmd, "cd") == 0;
}

void run_builtin(char **arguments)
{
    // echo command
    if (strcmp(arguments[0], "echo") == 0)
    {
        for (int i = 1; arguments[i] != NULL; i++)
        {
            if (i > 1)
                printf(" ");
            printf("%s", arguments[i]);
        }
        printf("\n");
    }

    // type command
    else if (strcmp(arguments[0], "type") == 0)
    {
        char *cmd = arguments[1];

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
    else if (strcmp(arguments[0], "pwd") == 0)
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
    else if (strcmp(arguments[0], "cd") == 0)
    {
        char *path = arguments[1];

        if (path == NULL)
        {
            printf("cd: missing argument\n");
            return;
        }

        if (strncmp(path, "~", 1) == 0)
        {
            char *home_dir = getenv("HOME");

            if (!home_dir)
            {
                printf("No home\n");
                return;
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
}