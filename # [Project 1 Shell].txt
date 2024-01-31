# [Project 1: Shell]

[The purpose of this project is to design and develop a comprehensive shell interface that enhances process control, user interaction, and error handling mechanisms. ]

## Group Members
- **[Jada Doby]**: [jdd20a@fsu.edu]
- **[Shelley Bercy]**: [sb22bg@fsu.edu]
- **[Rood Vilmont]**: [rkv20@fsu.edu]
## Division of Labor

### Part 1: [Prompt]
- **Responsibilities**: [Create a custom Bash prompt that displays the absolute working directory, username, and machine name. Users should be able to input commands on the same line as the prompt.]
- **Assigned to**: [Jada Doby, Rood Vilmont]

### Part 2: [Enviroment Variables]
- **Responsibilities**: [Develop a program that universally replaces tokens prefixed with the dollar sign character ($) with their respective values by leveraging the 'env' command and the getenv() function within the Bash shell.]
- **Assigned to**: [Jada Doby, Shelley Bercy]

### Part 3: [Tidle Expansion]
- **Responsibilities**: [Implement Bash tilde (~) expansion for paths, facilitating the transformation of tokens beginning with "~" or "~/".]
- **Assigned to**: [Shelley Bercy,Jada Doby]

### Part 4: [$PATH Search]
- **Responsibilities**: [Implement a Bash script that searches for executable commands in directories specified by the $PATH variable, providing proper error detection]
- **Assigned to**: [Shelley Bercy,Rood Vilmont]

### Part 5: [External Command Execution]
- **Responsibilities**: [In a Bash shell, use forking and execv() for the seamless execution of external commands, including handling multiple arguments, while maintaining shell stability.]
- **Assigned to**: [Rood Vilmont, Jada Doby]

### Part 6: [I/O Redirection]
- **Responsibilities**: [Implement input/output (I/O) redirection in the shell, enabling redirection of command output to files and reading command input from files, carefully considering file permissions and sequencing in complex scenarios.]
- **Assigned to**: [Rood Vilmont,Shelley Bercy]

### Part 7: [Piping]
- **Responsibilities**: [Enable piping functionality in the shell for the simultaneous execution of commands, facilitating the seamless flow of data between them and enhancing the flexibility of complex command sequences.]
- **Assigned to**: [Jada Doby, Rood Vilmont ]

### Part 8: [Background Processing]
- **Responsibilities**: [Integrate background processing in the shell for concurrent command execution, seamlessly working with I/O redirection and piping, and providing informative job status feedback.]
- **Assigned to**: [Shelley Bercy, Rood Vilmont]

### Part 9: [Internal Command Execution]
- **Responsibilities**: [Integrate internal commands in the shell, covering `exit` for managing background processes, `cd` for changing the current directory, and `jobs` for listing active background processes.]
- **Assigned to**: [Jada Doby, Shelley Bercy]


### Extra Credit
- **Responsibilities**: [Upgrade your shell to handle an unlimited number of pipes, support simultaneous piping and I/O redirection in a single command, and enable recursive execution for nested instances, showcasing advanced control flow within the shell environment.]
- **Assigned to**: [Jada Doby, Shelley Bercy, Rood Vilmont]

## File Listing
```
root/
â”‚
â”œâ”€â”€ src/
â”‚ â”œâ”€â”€ main.c
â”‚ â””â”€â”€ ...
â”‚
â”œâ”€â”€ include/
â”‚ â””â”€â”€ ...
â”‚
â”œâ”€â”€ README.md
â””â”€â”€ Makefile
```
## How to Compile & Execute

### Requirements
- **Compiler**: GCC
- **Dependencies**: List any libraries or frameworks necessary (rust only).

### Compilation
For a C/C++ example:
```bash
make
```
This will build the executable in ...
### Execution
```bash
make run
```
This will run the program ...

## Bugs
- **Bug 1**: This is bug 1.
- **Bug 2**: This is bug 2.
- **Bug 3**: This is bug 3.

## Extra Credit
- **Extra Credit 1:**: [Extra Credit Option]
- **Extra Credit 2:**: [Extra Credit Option]
- **Extra Credit 3:**: [Extra Credit Option]

## Considerations
[]
