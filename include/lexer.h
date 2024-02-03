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
    char **command;
} BackgroundProcess;

char *get_input(void);
tokenlist *get_tokens(char *input);
tokenlist *new_tokenlist(void);
void add_token(tokenlist *tokens, char *item);
void free_tokens(tokenlist *tokens);
void expand_tildes(tokenlist *tokens);
void Execute_Command(tokenlist *tokens, bool isBgProcess);
void addBGProcess(pid_t pid, const char **command, BackgroundProcess *bgProcesses);
void BackgroundProcessHelper(BackgroundProcess *bgProcesses);
void jobsCommand(bool background);
char *getPathSearch(tokenlist *cmd);
char *expand_tilde(const char *token);
void expand_tildeHelper(tokenlist *tokens);
void ioRedirection(tokenlist *tokens,bool isBgProcess);
tokenlist *expandEnvVariables(tokenlist *tokens);
void piping(tokenlist *tokens, bool isBgProcess);
void executePipedCommands(tokenlist *tokens);
