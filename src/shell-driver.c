#include "../include/lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
void Execute_Command(tokenlist *tokens);

int main()
{
    while (1)
    {
        printf("> "); // replace this with shell prompt in part 1
        char *input = get_input();
        tokenlist *tokens = get_tokens(input);
        External_Command(tokens);
        free(input);
        free_tokens(tokens);
    }

    return 0;
}
void Execute_Command(tokenlist *tokens)
{
    // need to use the implementation of $PATH Search in part 4 here
    pid_t process_id = fork();
    if (process_id == 0) // targeting child process
    {
        char **arguments = tokens->items;
        execvp(arguments[0], arguments);
    }
}
