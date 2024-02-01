#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



int main() {
    while (1) {
        // Getting username
        const char *username = getenv("USER");

        if (username != NULL) {
            // It's printing out later
        } else {
            perror("Error getting username");
            return 1;
        }

        // Getting machine name - using gethostname function
        char machine[300];
        if (gethostname(machine, sizeof(machine)) == 0) {
            // It's printing out later
        } else {
            perror("Error- Getting machine name");
            return 1; // indicates failure
        }

        // Getting working directory
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) == 0) {
            perror("Error - getting working directory");
            return 1;
        }

        // Print the prompt
        printf("%s@%s:%s>", username, machine, cwd);

        // Get user input and tokenize it
        char *input = get_input();
        printf("whole input: %s\n", input);

        tokenlist *tokens = expand_the_variables(input);
        for (int i = 0; i < tokens->size; i++) {
            printf("token %d: (%s)\n", i, tokens->items[i]);
        }

        free(input);
        free_tokens(tokens);
    }

    return 0;
}
