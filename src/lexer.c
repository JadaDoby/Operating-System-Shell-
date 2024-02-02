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
        strcpy(username,getenv("USER"));

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

       
        if(input[strlen(input)-1] == '&')
        {
            checkforBg = true;
            input[strlen(input)-1] = '\0';
        }

        printf("whole input: %s\n", input);

        // Expand environment variables in tokens
        tokenlist *tokens = get_tokens(input);
        expand_tildes(tokens);
        /*
        for (int i = 0; i < tokens->size; i++) {
            printf("token %d: (%s)\n", i, tokens->items[i]);
        }

       */
        if (result != NULL || result2 != NULL && !checkforBg)
        {
            ioRedirection(tokens,checkforBg);
        }
        else  if(strcmp(input,"jobs") == 0){
            jobsCommand(checkforBg);
        }
        else if(checkforBg)
        {
            Execute_Command(tokens, checkforBg);
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
        int execid = execv(getPathSearch(tokens), tokens->items);
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
           waitpid(pid, &status, WNOHANG);
           addBGProcess(pid, tokens->items[0], bgProcesses);
        }
        else
        {
            waitpid(pid, &status, 0);
        }
    }
}


void addBGProcess(pid_t pid, const char *command, BackgroundProcess *bgProcesses)
{
    if (globalJobCount < 10 && command != NULL) {
        bgProcesses[globalJobCount].pid = pid;
        bgProcesses[globalJobCount].command = (char *)malloc(strlen(command) + 1);
        strcpy(bgProcesses[globalJobCount].command, command);
        ++globalJobCount;
        printf("[%d] %d\n", globalJobCount, pid);
    } else {
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
            printf("[%d] done %s\n", i + 1, bgProcesses[i].command);

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
        }
    }
}

void jobsCommand(bool background)
{
    for (int i = 0; i < globalJobCount; i++)
    {
        if (bgProcesses[i].pid != 0)
        {
            pid_t pid = waitpid(bgProcesses[i].pid, NULL, WNOHANG);
            const char *status = (pid == bgProcesses[i].pid) ? "Done" : "Running";
            printf("[%d]+ %d %s\t%s\n", i + 1, bgProcesses[i].pid, status, bgProcesses[i].command);
        }
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
    const char *pathCopy = strdup(path); // Make a copy for tokenization

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

void expand_tildes(tokenlist *tokens)
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

// io redirection
void ioRedirection(tokenlist *tokens, bool isBgProcess)
{
	int size = tokens->size;
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
				Execute_Command(command,isBgProcess);
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
				Execute_Command(command,isBgProcess);
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
					fprintf(stderr, "no such file or directory: %s\n", tokens->items[i+1]);
					return;
				}
				int infd = dup(STDIN_FILENO);
				close(STDIN_FILENO);
				int fd = open(tokens->items[i + 1], O_RDWR , S_IRUSR);
				Execute_Command(command,isBgProcess);
				dup2(infd, STDIN_FILENO);
				close(infd);
				free_tokens(command);
				break;
			}
			else if(strcmp(tokens->items[i+2], ">") == 0){
				if (access(tokens->items[i+1], F_OK | R_OK) == -1)
				{
					fprintf(stderr, "no such file or directory: %s\n", tokens->items[i+1]);
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
				Execute_Command(command,isBgProcess);
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

// Function to expand tokens that start with '$'
char *expandvariabletokens(const char *token)
{
    if (token[0] == '$')
    {
        // Skip the '$' character
        const char *environmentname = token + 1;

        // Get the value of the environment variable
        const char *environmentvalue = getenv(environmentname);

        // If the environment variable is found, return its value
        if (environmentvalue != NULL)
        {
            return strdup(environmentvalue);
        }
    }
    // Return the token as is if not starting with '$' or not found in the environment
    return strdup(token);
}

tokenlist *expand_the_variables(char *input)
{
    // Allocate memory for a copy of the input string
    char *buf = (char *)malloc(strlen(input) + 1);
    strcpy(buf, input);

    // Create a new tokenlist to store the tokens
    tokenlist *tokens = new_tokenlist();

    // Tokenize the input string using space (' ') as a delimiter
    char *tok = strtok(buf, " ");
    while (tok != NULL)
    {
        // Expand tokens before adding to the list
        char *expanded_token = expandvariabletokens(tok);

        // Add the expanded token to the tokenlist
        add_token(tokens, expanded_token
        );

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


