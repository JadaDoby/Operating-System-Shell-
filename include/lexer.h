#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include<string.h>


typedef struct {
    char ** items;
    size_t size;
} tokenlist;

char * get_input(void);
tokenlist * get_tokens(char *input);
tokenlist * new_tokenlist(void);
void add_token(tokenlist *tokens, char *item);
void free_tokens(tokenlist *tokens);

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



// Function to search for a command in the directories listed in $PATH
char *getPathSearch(tokenlist *cmd)
{

    char *path = NULL;
    path = (char *)malloc(sizeof(char) * (strlen(getenv("PATH")) + 1));
    strcpy(path, getenv("PATH"));
    char *tokens = strtok(path, ":");

    while (tokens != NULL)
    {

        if (tokens[0] == '~')
        {
            char fullTokenPath[(strlen(tokens) - 1) + (strlen(getenv("HOME"))) + 2];
            strcpy(fullTokenPath, expand_tilde(tokens));
            tokens = (char *)malloc(sizeof(char) * strlen(fullTokenPath) + 1);
            strcpy(tokens, fullTokenPath);
        }

        char *slash = "/";
        int size = strlen(tokens);
        int cmdSize = strlen(cmd->items[0]);
        int slashSize = strlen(slash);
        int fullPathSize = size + cmdSize + slashSize + 1;
        char *temp = (char *)malloc(sizeof(char) * fullPathSize);
        temp[0] = '\0';
        strcat(temp, tokens);

        strcat(temp, slash);
        strcat(temp, cmd->items[0]);
        if (access(temp, F_OK | R_OK) != -1)
        {
            return temp;
        }
        tokens = strtok(NULL, ":");
    }

    printf("%s\n", "Command Not Found");
    return NULL;
}

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
