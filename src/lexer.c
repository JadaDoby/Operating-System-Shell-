#include "../include/lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int globalJobCount = 0;
BackgroundProcess bgProcesses[10];

int main()
{
    while (1)
    {
        // Getting username
        char *username = (char *)malloc(sizeof(char) * (strlen(getenv("USER")) + 1));
        strcpy(username, getenv("USER"));

        if (username != NULL)
        {
            // It's printing out later
        }
        else
        {
            perror("Error getting username");
            return 1;
        }

        // Getting machine name - using gethostname function
        char machine[300];
        if (gethostname(machine, sizeof(machine)) == 0)
        {
            // It's printing out later
        }
        else
        {
            perror("Error- Getting machine name");
            return 1; // indicates failure
        }

        // Getting working directory
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) == 0)
        {
            perror("Error - getting working directory");
            return 1;
        }

        // Print the prompt
        printf("%s@%s:%s>", username, machine, cwd);

        // Get user input and tokenize it
        char *input = get_input();
        char checkforIO = '<';
        char checkforIO2 = '>';
        char checkfortilde = '~';
        bool checkforBg = false;
        const char *result = strchr(input, checkforIO);
        const char *result2 = strchr(input, checkforIO2);

        if (input[strlen(input) - 1] == '&')
        {
            checkforBg = true;
            input[strlen(input) - 1] = '\0';
        }

        // printf("whole input: %s\n", input);

        // Expand environment variables in tokens
        tokenlist *tokens = get_tokens(input);
        expand_tildeHelper(tokens);
        expandEnvVariables(tokens);

         if (strcmp(tokens->items[0], "cd") == 0)
        {
                cdCommand(tokens);
                continue;
        }

        int hasPipe = 0;
        for (int i = 0; i < tokens->size; i++)
        {
            if (strcmp(tokens->items[i], "|") == 0)
            {
                hasPipe = 1;
                break;
            }
        }

        if (result != NULL || result2 != NULL && !checkforBg)
        {
            ioRedirection(tokens, checkforBg);
        }
        else if (hasPipe)
        {
            piping(tokens, checkforBg);
        }
        else if (strcmp(input, "jobs") == 0)
        {
            jobsCommand(checkforBg);
        }
        else if (checkforBg)
        {
            if (result != NULL || result2 != NULL)
            {
                ioRedirection(tokens, checkforBg);
            }
            else if (hasPipe)
            {
                piping(tokens, checkforBg);
            }
            else
            {
                Execute_Command(tokens, checkforBg);
            }
        }
        else
        {
            Execute_Command(tokens, checkforBg);
        }

        // Free resources
        free(username);
        free(input);
        free_tokens(tokens);
        BackgroundProcessHelper(bgProcesses);
    }

    return 0;
}

void Execute_Command(tokenlist *tokens, bool isBgProcess)
{
    pid_t pid;
    int status;
    pid = fork();
    if (pid == 0)
    {
        char *path = getPathSearch(tokens);
        if (path == NULL)
        {
            exit(EXIT_FAILURE);
        }
        int execid = execv(path, tokens->items);
        if (execid == -1)
        {
            perror("Error");
            exit(EXIT_FAILURE);
        }
    }
    else if (pid < 0)
    {
        perror("Error forking");
    }
    else
    {
        if (isBgProcess)
        {
            char *temp[] = {tokens->items[0], NULL};
            waitpid(pid, &status, WNOHANG);

            addBGProcess(pid, temp, bgProcesses);
            printf("[%d] %d\n", globalJobCount, pid);
        }
        else
        {
            waitpid(pid, &status, 0);
        }
    }
}

// ls as
// ls al

void addBGProcess(pid_t pid, char **command, BackgroundProcess *bgProcesses)
{
    int size = 0;
    while (command[size] != NULL)
    {
        size++;
    }
    if (globalJobCount < 10 && command != NULL)
    {
        bgProcesses[globalJobCount].pid = pid;

        // Allocate memory for the command array
        bgProcesses[globalJobCount].command = (char **)malloc((size + 1) * sizeof(char *));

        for (int i = 0; i < size; i++)
        {
            // Allocate memory for each command string
            bgProcesses[globalJobCount].command[i] = (char *)malloc(strlen(command[i]) + 1);

            strcpy(bgProcesses[globalJobCount].command[i], command[i]);
        }

        // Set the last element of the command array to NULL
        bgProcesses[globalJobCount].command[size] = NULL;

        ++globalJobCount;
    }
    else
    {
        fprintf(stderr, "Error:");
    }
}

void BackgroundProcessHelper(BackgroundProcess *bgProcesses)
{
    int status;
    pid_t pid;

    for (int i = 0; i < globalJobCount; ++i)
    {
        pid = waitpid(bgProcesses[i].pid, &status, WNOHANG);
        if (pid > 0)
        {
            printf("[%d]+ done", i + 1);
            for (int j = 0; bgProcesses[i].command[j] != NULL; ++j)
            {
                printf(" %s|", bgProcesses[i].command[j]);
            }
            printf("\n");

            if (i >= 0 && i < globalJobCount)
            {
                free(bgProcesses[i].command);
                for (int j = i; j < globalJobCount - 1; ++j)
                {
                    bgProcesses[j] = bgProcesses[j + 1];
                }
                --globalJobCount;
            }
        }
        else if (pid < 0)
        {
            perror("Error waiting for background process");
            return;
        }
    }
}

void jobsCommand(bool background)
{
    if (globalJobCount == 0)
    {
        printf("No background jobs\n");
        return;
    }
    for (int i = 0; i < globalJobCount; i++)
    {
        if (bgProcesses[i].pid != 0)
        {
            pid_t pid = waitpid(bgProcesses[i].pid, NULL, WNOHANG);
            const char *status = (pid == bgProcesses[i].pid) ? "Done" : "Running";
            printf("[%d]+ %d %s\t%s\n", i + 1, bgProcesses[i].pid, status, bgProcesses[i].command[0]);
        }
    }
}

void cdCommand(tokenlist *token) {
    char *newDir;
    if(token->size > 2){
        fprintf(stderr, "cd: More than one argument\n");
        return;
    }
    if (token->size == 1) {
        char *home = getenv("HOME");
        newDir = strdup(home);
        if (newDir == NULL) {
            perror("cd");
            return;
        }
    } else {
        newDir = strdup(token->items[1]);
        if (newDir == NULL) {
            perror("cd");
            return;
        }
    }

    if (access(newDir, F_OK) != 0) {
        fprintf(stderr, "cd: %s: No such file or directory\n", newDir);
        free(newDir);
        return;
    }

    if (chdir(newDir) != 0) {
        perror("cd");
        free(newDir);
        return;
    }

    if (setenv("PWD", newDir, 1) != 0) {
        perror("error");
    }

    free(newDir);
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
    tokens->items[0] = NULL;
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
    char *pathCopy = strdup(path);

    char *tokens = strtok(pathCopy, ":");

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
            free((void *)pathCopy);
            return fullPath;
        }

        free(temp);
        free(fullPath);
        tokens = strtok(NULL, ":");
    }

    printf("%s\n", "Command Not Found");
    free(path);
    free((void *)pathCopy);
    return NULL;
}

void expand_tildeHelper(tokenlist *tokens)
{
    for (int i = 0; i < tokens->size; i++)
    {
        if (tokens->items[i][0] == '~')
        {
            char *expandedTilde = expand_tilde(tokens->items[i]);
            free(tokens->items[i]);
            tokens->items[i] = expandedTilde;
        }
    }
}

char *expand_tilde(const char *token)
{
    char *path = NULL;
    if (strcmp(token, "~") == 0 && strlen(token) == 1)
    {
        path = (char *)malloc(sizeof(char) * (strlen(getenv("HOME")) + 1));
        strcpy(path, getenv("HOME"));
    }
    else if (strncmp(token, "~/", 2) == 0)
    {
        const char *homeEnv = getenv("HOME");
        const char *remainingPath = token + 1;
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

// io redirection
void ioRedirection(tokenlist *tokens, bool isBgProcess)
{
    int size = tokens->size;
    if (size < 3)
    {
        perror("Error: no file");
        return;
    }
    tokenlist *command = new_tokenlist();
    for (int i = 0; i < size; i++)
    {

        if (strcmp(tokens->items[i], ">") != 0 && strcmp(tokens->items[i], "<") != 0)
        {
            add_token(command, tokens->items[i]);
        }
        if (strcmp(tokens->items[i], ">") == 0)
        {
            if (i + 1 == size - 1)
            {
                int outfd = dup(STDOUT_FILENO);
                close(STDOUT_FILENO);
                int fd = open(tokens->items[i + 1], O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
                Execute_Command(command, isBgProcess);
                dup2(outfd, STDOUT_FILENO);
                close(outfd);
                free_tokens(command);
                break;
            }
            else if (strcmp(tokens->items[i + 2], "<") == 0)
            {
                if (access(tokens->items[i + 3], F_OK | R_OK) == -1)
                {
                    fprintf(stderr, "no such file or directory: %s\n", tokens->items[i + 3]);
                    return;
                }
                int infd = dup(STDIN_FILENO);
                close(STDIN_FILENO);
                int fd = open(tokens->items[i + 3], O_RDWR, S_IRUSR);
                dup2(infd, STDIN_FILENO);
                close(infd);
                int outfd = dup(STDOUT_FILENO);
                close(STDOUT_FILENO);
                int fd2 = open(tokens->items[i + 1], O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
                Execute_Command(command, isBgProcess);
                dup2(outfd, STDOUT_FILENO);
                close(outfd);
                free_tokens(command);
                break;
            }
            else
            {
                perror("Error: no file");
                return;
            }
        }
        if (strcmp(tokens->items[i], "<") == 0)
        {
            if (i + 1 == size - 1)
            {
                if (access(tokens->items[i + 1], F_OK | R_OK) == -1)
                {
                    fprintf(stderr, "no such file or directory: %s\n", tokens->items[i + 1]);
                    return;
                }
                int infd = dup(STDIN_FILENO);
                close(STDIN_FILENO);
                int fd = open(tokens->items[i + 1], O_RDWR, S_IRUSR);
                Execute_Command(command, isBgProcess);
                dup2(infd, STDIN_FILENO);
                close(infd);
                free_tokens(command);
                break;
            }
            else if (strcmp(tokens->items[i + 2], ">") == 0)
            {
                if (access(tokens->items[i + 1], F_OK | R_OK) == -1)
                {
                    fprintf(stderr, "no such file or directory: %s\n", tokens->items[i + 1]);
                    return;
                }
                int infd = dup(STDIN_FILENO);
                close(STDIN_FILENO);
                int fd = open(tokens->items[i + 1], O_RDWR, S_IRUSR);
                dup2(infd, STDIN_FILENO);
                close(infd);
                int outfd = dup(STDOUT_FILENO);
                close(STDOUT_FILENO);
                int fd2 = open(tokens->items[i + 3], O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
                Execute_Command(command, isBgProcess);
                dup2(outfd, STDOUT_FILENO);
                close(outfd);
                free_tokens(command);
                break;
            }
            else
            {
                perror("Error");
                return;
            }
        }
    }
}

tokenlist *expandEnvVariables(tokenlist *tokens)
{
    int size = tokens->size;
    char *temp = NULL;
    for (int i = 0; i < size; i++)
    {
        if (tokens->items[i][0] == '$')
        {
            temp = (char *)malloc(sizeof(char) * (strlen(getenv(tokens->items[i] + 1)) + 1));
            strcpy(temp, getenv(tokens->items[i] + 1));
            tokens->items[i] = temp;
        }
    }
    return tokens;
}

void piping(tokenlist *tokens, bool isBgProcess)
{
    tokenlist *cmd1 = new_tokenlist();
    int index = 0;
    tokenlist *cmd2 = new_tokenlist();
    tokenlist *cmd3 = new_tokenlist();

    for (int i = 0; i < tokens->size; i++)
    {
        index++;
        if (strcmp(tokens->items[i], "|") != 0)
        {
            add_token(cmd1, tokens->items[i]);
        }
        else
        {
            break;
        }
    }

    for (int i = index; i < tokens->size; i++)
    {
        index++;
        if (strcmp(tokens->items[i], "|") != 0)
        {
            add_token(cmd2, tokens->items[i]);
        }
        else
        {
            break;
        }
    }

    int fd[2];
    int fd2[2];
    if (pipe(fd) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    if (index < tokens->size)
    {
        if (pipe(fd2) == -1)
        {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    pid_t pid1 = fork();
    if (pid1 < 0)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid1 == 0)
    {
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]);
        close(fd[1]);
        execv(getPathSearch(cmd1), cmd1->items);
    }

    pid_t pid2 = fork();
    if (pid2 < 0)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid2 == 0)
    {
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);
        close(fd[1]);
        if (index < tokens->size)
        {
            dup2(fd2[1], STDOUT_FILENO);
            close(fd2[0]);
            close(fd2[1]);
        }
        execv(getPathSearch(cmd2), cmd2->items);
    }

    pid_t pid3;
    if (index < tokens->size)
    {
        free_tokens(cmd3);
        cmd3 = new_tokenlist();

        for (int i = index; i < tokens->size; i++)
        {
            if (strcmp(tokens->items[i], "|") != 0)
            {
                add_token(cmd3, tokens->items[i]);
            }
        }
        pid3 = fork();
        if (pid3 < 0)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if (pid3 == 0)
        {
            dup2(fd2[0], STDIN_FILENO);
            close(fd2[0]);
            close(fd2[1]);
            execv(getPathSearch(cmd3), cmd3->items);
        }
    }

    char *temp[] = {cmd1->items[0], cmd2->items[0], NULL};

    if (isBgProcess)
    {
        addBGProcess(pid1, temp, bgProcesses);
        printf("[%d] %d\n", globalJobCount, pid2);

        if (index < tokens->size)
        {
            char *temp1[] = {cmd2->items[0], cmd3->items[0], NULL};

            addBGProcess(pid3, temp1, bgProcesses);
        }
    }
    close(fd[0]);
    close(fd[1]);
    if (index < tokens->size)
    {
        close(fd2[0]);
        close(fd2[1]);
    }
    if (isBgProcess)
    {
        waitpid(pid1, NULL, WNOHANG);
        waitpid(pid2, NULL, WNOHANG);
        if (index < tokens->size)
        {
            waitpid(pid3, NULL, WNOHANG);
        }
    }
    else
    {
        waitpid(pid1, NULL, 0);
        waitpid(pid2, NULL, 0);
        if (index < tokens->size)
        {
            waitpid(pid3, NULL, 0);
        }
    }
}
