#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

typedef struct
{
    char **items;
    size_t size;
} tokenlist;

typedef struct
{
    pid_t pid;
    char *command;
} BackgroundProcess;

char *get_input(void);
tokenlist *get_tokens(char *input);
tokenlist *new_tokenlist(void);
void add_token(tokenlist *tokens, char *item);
void free_tokens(tokenlist *tokens);
void expand_tildes(tokenlist *tokens);
void Execute_Command(tokenlist *tokens, bool isBgProcess);
char *getPathSearch(tokenlist *cmd);
char *expand_tilde(const char *token);
void ioRedirection(tokenlist *tokens,bool isBgProcess);
tokenlist *expand_the_variables(tokenlist *tokens);
void pipingcommand(tokenlist *tokens);
tokenlist *expand_the_variables(tokenlist* tokens)
{
    int size = tokens->size;
    char* temp = NULL;

    for (int i = 0; i < size; i++)
    {
        if(tokens->items[i][0]=='$'){
            temp = (char*)malloc(sizeof(char)*strlen(getenv(tokens->items[i][0])) + 1);
            temp->items[i] = (char*)malloc(sizeof(char)*strlen(getenv(tokens->items[i][0])) + 1);
            strcpy(temp,getenv(tokens->items[i][0]));
            strcpy(temp->items,temp);
        }
    }
void pipingcommand(tokenlist *tokens)
{
    bool isBgProcess = false;
    int pipefd[2];
    pid_t pid1, pid2;
    int status1, status2;

    tokenlist *cmd1 = new_tokenlist();
    tokenlist *cmd2 = new_tokenlist();

    int pipeIndex = 0;
    for (int i = 0; i < tokens->size; i++)
    {
        if (strcmp(tokens->items[i], "|") == 0)
        {
            pipeIndex = i;
            break;
        }
        add_token(cmd1, tokens->items[i]);
    }

    for (int i = pipeIndex + 1; i < tokens->size; i++)
    {
        add_token(cmd2, tokens->items[i]);
    }

    // Create a pipe
    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // First child process
    pid1 = fork();
    if (pid1 == 0)
    {
        close(pipefd[0]); // Close read end of the pipe

        // Redirect stdout to the write end of the pipe
        dup2(pipefd[1], STDOUT_FILENO);

        close(pipefd[1]); // Close unused file descriptors

        // Execute cmd1
        if (Execute_Command(cmd1, isBgProcess) == -1)
        {
            perror("Execute_Command (cmd1)");
            exit(EXIT_FAILURE);
        }

        exit(EXIT_SUCCESS);
    }

    // Second child process
    pid2 = fork();
    if (pid2 == 0)
    {
        close(pipefd[1]); // Close write end of the pipe

        // Redirect stdin to the read end of the pipe
        dup2(pipefd[0], STDIN_FILENO);

        close(pipefd[0]); // Close unused file descriptors

        // Execute cmd2
        if (Execute_Command(cmd2, isBgProcess) == -1)
        {
            perror("Execute_Command (cmd2)");
            exit(EXIT_FAILURE);
        }

        exit(EXIT_SUCCESS);
    }

    // Close unused file descriptors in the parent process
    close(pipefd[0]);
    close(pipefd[1]);

    // Wait for both child processes to finish
    waitpid(pid1, &status1, 0);
    waitpid(pid2, &status2, 0);

    // Free memory
    free_tokens(cmd1);
    free_tokens(cmd2);
}




void pipingcommand(tokenlist *tokens)
{
    bool isBgProcess = false;
    pid_t pid;
    int status;

    int pipes[3][2];  // Three pipes for three commands
    tokenlist *cmd1 = new_tokenlist();
    tokenlist *cmd2 = new_tokenlist();
    tokenlist *cmd3 = new_tokenlist();

    // Find the indices of the pipe symbols
    int pipeIndices[2] = {0, 0};
    for (int i = 0; i < tokens->size; i++)
    {
        if (strcmp(tokens->items[i], "|") == 0)
        {
            if (pipeIndices[0] == 0)
            {
                pipeIndices[0] = i;
            }
            else
            {
                pipeIndices[1] = i;
                break;
            }
        }
        add_token(cmd1, tokens->items[i]);
    }

    // Populate cmd2 with tokens between the first and second pipe symbols
    for (int i = pipeIndices[0] + 1; i < pipeIndices[1]; i++)
    {
        add_token(cmd2, tokens->items[i]);
    }

    // Populate cmd3 with tokens after the second pipe symbol
    for (int i = pipeIndices[1] + 1; i < tokens->size; i++)
    {
        add_token(cmd3, tokens->items[i]);
    }

    // Create pipes
    if (pipe(pipes[0]) == -1 || pipe(pipes[1]) == -1 || pipe(pipes[2]) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // First child process
    pid = fork();
    if (pid == 0)
    {
        // Close read ends of the pipes
        close(pipes[0][0]);
        close(pipes[1][0]);
        close(pipes[1][1]);
        close(pipes[2][0]);
        close(pipes[2][1]);

        // Redirect stdout to the write end of the first pipe
        dup2(pipes[0][1], STDOUT_FILENO);

        // Close unused file descriptors
        close(pipes[0][1]);

        // Execute cmd1
        Execute_Command(cmd1, isBgProcess);

        // Exit child process
        exit(EXIT_SUCCESS);
    }

    // Second child process
    pid = fork();
    if (pid == 0)
    {
        // Close read end of the first pipe and write end of the second pipe
        close(pipes[0][1]);
        close(pipes[1][0]);
        close(pipes[2][0]);
        close(pipes[2][1]);

        // Redirect stdin from the read end of the first pipe
        dup2(pipes[0][0], STDIN_FILENO);

        // Close read end of the first pipe
        close(pipes[0][0]);

        // Redirect stdout to the write end of the second pipe
        dup2(pipes[1][1], STDOUT_FILENO);

        // Close unused file descriptors
        close(pipes[1][1]);

        // Execute cmd2
        Execute_Command(cmd2, isBgProcess);

        // Exit child process
        exit(EXIT_SUCCESS);
    }

    // Third child process
    pid = fork();
    if (pid == 0)
    {
        // Close read end of the second pipe
        close(pipes[0][0]);
        close(pipes[1][1]);
        close(pipes[2][0]);

        // Redirect stdin from the read end of the second pipe
        dup2(pipes[1][0], STDIN_FILENO);

        // Close read end of the second pipe
        close(pipes[1][0]);

        // Execute cmd3
        Execute_Command(cmd3, isBgProcess);

        // Exit child process
        exit(EXIT_SUCCESS);
    }

    // Close unused file descriptors in the parent process
    close(pipes[0][0]);
    close(pipes[0][1]);
    close(pipes[1][0]);
    close(pipes[1][1]);
    close(pipes[2][1]);

    // Wait for all child processes to finish
    waitpid(-1, &status, 0);

    // Close read end of the third pipe
    close(pipes[2][0]);

    // Read and print the output of the last command
    char buffer[1024];
    ssize_t bytesRead;
    while ((bytesRead = read(pipes[2][0], buffer, sizeof(buffer))) > 0)
    {
        write(STDOUT_FILENO, buffer, bytesRead);
    }

    // Close the read end of the third pipe
    close(pipes[2][0]);

    // Free memory
    free_tokens(cmd1);
    free_tokens(cmd2);
    free_tokens(cmd3);
}
