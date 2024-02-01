#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include<string.h>
#include <sys/types.h>
//#include <bits/waitflags.h>
#include <sys/wait.h>


typedef struct {
    char ** items;
    size_t size;
} tokenlist;

char * get_input(void);
tokenlist *get_tokens(char *input);
tokenlist * new_tokenlist(void);
void add_token(tokenlist *tokens, char *item);
void free_tokens(tokenlist *tokens);


void Execute_Command(tokenlist *tokens);
char *getPathSearch(tokenlist *cmd);
char *expand_tilde(const char *token);




void Execute_Command(tokenlist *tokens){
    pid_t pid;
    int status;
    pid = fork();
    if (pid == 0) {
        // Child process
        if (execv(getPathSearch(tokens), tokens->items) == -1) {
            perror("Error executing command");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        perror("Error forking");
    } else {
        // Parent process
        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
}
}

char *get_input(void)
{
    char *buffer = NULL;
    int bufsize = 0;
    char line[5];
    while (fgets(line, 5, stdin) != NULL)
    {
        int addby = 0;
        char *newln = strchr(line, '\n');
        if (newln != NULL)
            addby = newln - line;
        else
            addby = 5 - 1;
        buffer = (char *)realloc(buffer, bufsize + addby);
        memcpy(&buffer[bufsize], line, addby);
        bufsize += addby;
        if (newln != NULL)
            break;
    }
    buffer = (char *)realloc(buffer, bufsize + 1);
    buffer[bufsize] = 0;
    return buffer;
}

tokenlist *new_tokenlist(void)
{
    tokenlist *tokens = (tokenlist *)malloc(sizeof(tokenlist));
    tokens->size = 0;
    tokens->items = (char **)malloc(sizeof(char *));
    tokens->items[0] = NULL; /* make NULL terminated */
    return tokens;
}

void add_token(tokenlist *tokens, char *item)
{
    int i = tokens->size;

    tokens->items = (char **)realloc(tokens->items, (i + 2) * sizeof(char *));
    tokens->items[i] = (char *)malloc(strlen(item) + 1);
    tokens->items[i + 1] = NULL;
    strcpy(tokens->items[i], item);

    tokens->size += 1;
}

tokenlist *get_tokens(char *input)
{
    char *buf = (char *)malloc(strlen(input) + 1);
    strcpy(buf, input);
    tokenlist *tokens = new_tokenlist();
    char *tok = strtok(buf, " ");
    while (tok != NULL)
    {
        add_token(tokens, tok);
        tok = strtok(NULL, " ");
    }
    free(buf);
    return tokens;
}

void free_tokens(tokenlist *tokens)
{
    for (int i = 0; i < tokens->size; i++)
        free(tokens->items[i]);
    free(tokens->items);
    free(tokens);
}
char *getPathSearch(tokenlist *cmd)
{
    char *path = NULL;
    path = (char *)malloc(sizeof(char) * (strlen(getenv("PATH")) + 1));
    strcpy(path, getenv("PATH"));
    const char *pathCopy = strdup(path);  // Make a copy for tokenization

    const char *tokens = strtok(pathCopy, ":");

    while (tokens != NULL)
    {
        char *temp;
        if (tokens[0] == '~')
        {
            char fullTokenPath[(strlen(tokens) - 1) + (strlen(getenv("HOME"))) + 2];
            strcpy(fullTokenPath, expand_tilde(tokens));
            temp = (char *)malloc(sizeof(char) * (strlen(fullTokenPath) + 1));
            strcpy(temp, fullTokenPath);
        }
        else
        {
            temp = strdup(tokens);
        }

        char *slash = "/";
        int size = strlen(temp);
        int cmdSize = strlen(cmd->items[0]);
        int slashSize = strlen(slash);
        int fullPathSize = size + cmdSize + slashSize + 1;
        char *fullPath = (char *)malloc(sizeof(char) * fullPathSize);
        fullPath[0] = '\0';
        strcat(fullPath, temp);
        strcat(fullPath, slash);
        strcat(fullPath, cmd->items[0]);

        if (access(fullPath, F_OK | R_OK) != -1)
        {
            free(path);
            free(temp);
            free(pathCopy);
            return fullPath;
        }

        free(temp);
        free(fullPath);
        tokens = strtok(NULL, ":");
    }

    printf("%s\n", "Command Not Found");
    free(path);
    free(pathCopy);
    return NULL;
}

//
char *expand_tilde(const char *token)
{
    char *path = NULL;
    if (strcmp(token, "~") == 0)
    {
        // If the token is "~", expand it to the home directory
        path = (char *)malloc(sizeof(char) * (strlen(getenv("HOME")) + 1));
        strcpy(path, getenv("HOME"));
    }
    else if (strncmp(token, "~/", 2) == 0)
    {
        // If the token starts with "~/", expand it to the home directory + remaining path
        const char *homeEnv = getenv("HOME");
        const char *remainingPath = token + 1; // Skip the "~/"
        int fullLength = strlen(homeEnv) + strlen(remainingPath) + 1;
        path = (char *)malloc(sizeof(char) * fullLength);
        strcpy(path, homeEnv);
        strcat(path, remainingPath);
    }
    else
    {
        path = strdup(token);
    }

    return path;
}

void redirection(tokenlist *cmd1, tokenlist *cmd2) {
    int pipefd[2];
    pid_t pid;

    // create a pipe
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // fork the 1st process
    pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // Child process (cmd1)
        // Close the write end of the pipe (cmd1 only reads)
        close(pipefd[1]);

        // Redirect standard input to read from the pipe
        dup2(pipefd[0], STDIN_FILENO);

        // Close unused file descriptors
        close(pipefd[0]);

        // Execute cmd1
        if (execv(cmd1->items[0], cmd1->items) == -1) {
            perror("Error executing cmd1");
            exit(EXIT_FAILURE);
        }
    } else { // Parent process
        // Close the read end of the pipe (parent only writes)
        close(pipefd[0]);

        // Fork the 2nd process
        pid = fork();

        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) { // Child process (cmd2)
            // Redirect standard output to write to the pipe
            dup2(pipefd[1], STDOUT_FILENO);

            // Close unused file descriptors
            close(pipefd[1]);

            // Execute cmd2
            if (execv(cmd2->items[0], cmd2->items) == -1) {
                perror("Error executing cmd2");
                exit(EXIT_FAILURE);
            }
        } else { // Parent process
            // Close the write end of the pipe (parent only reads)
            close(pipefd[1]);

            // Wait for both child processes to finish
            wait(NULL);
            wait(NULL);
        }
    }
}

// Function to expand tokens that start with '$'
char *expandvariabletokens(const char *token) {
    if (token[0] == '$') {
        // Skip the '$' character
        const char *environmentname = token + 1;

        // Get the value of the environment variable
        const char *environmentvalue = getenv(environmentname);

        // If the environment variable is found, return its value
        if (environmentvalue != NULL) {
            return strdup(environmentvalue);
        }
    }
    // Return the token as is if not starting with '$' or not found in the environment
    return strdup(token);
}

tokenlist *expand_the_variables(char *input) {
    // Allocate memory for a copy of the input string
    char *buf = (char *)malloc(strlen(input) + 1);
    strcpy(buf, input);

    // Create a new tokenlist to store the tokens
    tokenlist *tokens = new_tokenlist();

    // Tokenize the input string using space (' ') as a delimiter
    char *tok = strtok(buf, " ");
    while (tok != NULL) {
        // Expand tokens before adding to the list
        char *expanded_token = expandvariabletokens(tok);

        // Add the expanded token to the tokenlist
        add_token(tokens, expanded_token);

        // Print intermediate values for debugging
        printf("Original Token: %s, Expanded Token: %s\n", tok, expanded_token);

        // Free the memory allocated for the expanded token
        free(expanded_token);

        // Move to the next token
        tok = strtok(NULL, " ");
    }

    // Free the memory allocated for the input string copy
    free(buf);

    // Return the tokenlist containing the expanded tokens
    return tokens;
}
