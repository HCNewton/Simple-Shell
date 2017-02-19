/************************************************
 *          Conor Newton - 130327088            *
 ************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define tok_delim " \t\r\n\a"                                    // tok_delim is a delimiter used during a strtok() function

/*  Built in shell comand functions are declared */

int builtin_cd(char **args);
int builtin_help(char **args);
int builtin_exit(char **args);

/*  The char "line" holds the last line entered by the user */
char *line;

/*  The names of the built in commands are stored in the builtin_calls array */
char *builtin_calls[] =
{
    "cd",
    "help",
    "exit"
};

/*
    builtin_func array holds a reference to each of the commands.
    The forward declaration of the functions allows them to be declared before they are defined.
*/
int (*builtin_func[])(char **) =
{
    &builtin_cd,
    &builtin_help,
    &builtin_exit
};


int builtin_cd(char **args)                                     // args[0] is "cd" and args[1] is the directory
{
    if (args[1] == NULL)                                        // The function first checks for the second argument
    {
        if (chdir("..") != 0)                                   // If no argument is found then the directory is moved up the path
        {
            perror("cd");                                       // An error is issued if the the previous path cannot be loaded
        }
    }
    else
    {
        if (chdir(args[1]) != 0)                                // When an argument is found the directory is changed to that of the argument
        {
            perror("cd");                                       // If the argument is invalid and the file directory cannot be found, than an error is issued
        }
    }
    return 1;                                                   // 1 is always issued in order to continue executing
}

/*
    When the help command is entered, the user is provided with information about the program.
 */
int builtin_help(char **args)
{
    printf("A Basic Shell, by Conor Newton.\n");
    printf("Type program names and arguments, then hit enter.\n");
    printf("Use the \"man\" command with an argument for information on other programs.\n");
    printf("Most of the basic Unix commands have been implemented.\n");
    printf("\"help\" and \"cd\" have been built in.\n");
    printf("Enter \"exit\" when you are finished.\n");

    return 1;                                                   // 1 is returned to allow to program to continue
}
/*
    when the exit command is entered, the shell is terminated.
*/
int builtin_exit(char **args)
{
    return 0;                                                   //when a 0 is returned the shell stops reading and executing
}

int launch(char **args)
{
    pid_t pid, wpid;
    int status;

    /*
         when fork() is returned, there are two of the same processes running simultaneously, child and parent processes
    */
    pid = fork();                                               // Fork returns a process ID number
    if (pid == 0)                                               // The child process is returned as 0
    {
        if (execvp(args[0], args) == -1)                        // "execvp" replaces the child process with the user determined program in argument 0
        {
            perror(line);                                       // If -1 is returned, the forking has failed and the user is alerted of the error
        }
        exit(EXIT_FAILURE);
    }
    else if (pid < 0)                                           // If the process ID number returns less than 0, there has been an error forking the process
    {
        perror(line);                                           // The user is alerted of the error and expected to handle it themselves
    }
    else
    {
        do
        {
            wpid = waitpid(pid, &status, WUNTRACED);            // Waitpid is used to allow the parent process the wait for the child process's state to change
        }
        /*
            WIFEXITED(status) returns True if status was returned for a child that terminated normally.
            WIFSIGNALED(status) returns True if status was returned for a child that terminated abnormally.
            so the loop continues to run as long as both statuses remain untrue
        */
        while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;                                                   // 1 is returned to allow the program to continue executing after launch
}

int execute(char **args)
{
    int i;

    if (args[0] == NULL)                                        // If no command is entered, the program continues as normal
    {
        // args[0] is the line entered by the user
        return 1;                                               // 1 is returned to allow to program to continue
    }

    for (i = 0; i < 3; i++)                                     // For loop runs though all builtin calls to see if they were called
    {
        if (strcmp(args[0], builtin_calls[i]) == 0)             // The string entered by the user is compared to the builtin_calls
        {
            return (*builtin_func[i])(args);                    // If they match, the corresponding function is called
        }
    }

    return launch(args);                                        // The argument is then passed to launch, the running process is forked and the argument takes the place of the child process
}

char *read_line(void)
{
    char *line = NULL;                                          // Line is set to null, ready to hold the users input
    ssize_t bufsize = 0;                                        // A buffer is allocated for getline, to determine the streamsize
    /*
     * Getline has 3 parameters, the first "&line" uses the & operator to refer to the pointer for the char line.
     * The &bufsize determines the maximum amount of characters that need to be written.
     * The 3rd parameter determines when the getline function should stop extracting characters, in this case
     * standard input (stdin) is used meaning getline finishes when all the characters entered into the shell have been read
     */
    getline(&line, &bufsize, stdin);
    return line;
}


char **split_line(char *line)

/*
 * This function is used in order to parse the user entered line into a list of arguments.
 * The arguments are separated in this program by a blank space.
 * Meaning "an argument" will be read as two separate arguments instead of one.
 */

{
    int bufsize = 64, i = 0;                             // local variables are initialised. bufsize determines the amount of space allocated for the buffer
    char **tokens = malloc(bufsize * sizeof(char*));            // Malloc allocates a block of size bytes of memory, returning a pointer to the beginning of the block
    char *token;

    if (!tokens)                                                // If the line cannot be read than an error is issued to the used
    {
        fprintf(stderr, "My Shell: allocation error\n");        // stderr is the standard I/O error stream
        exit(EXIT_FAILURE);
    }

    token = strtok(line, tok_delim);                            // strtok breaks the char "line" into a series of tokens using the delimiter, these tokens are then set to the char token
                                                                // "line" holds the users entered input from the keyboard

    while (token != NULL)                                       // When a line has been entered this loop will function. The process repeats until no token is returned by strtok
    {
        tokens[i] = token;
        i++;

        if (i >= bufsize)                                       // If the line exceeds the buffer size, then more memory is added
        {
            bufsize += 64;
            tokens = realloc(tokens, bufsize * sizeof(char*));  // realloc works the same as malloc, but instead of creating it the memory, it allows you to change the size
            if (!tokens)                                        // Again, if the line cannot be read than an error is reported
            {
                fprintf(stderr, "My Shell: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, tok_delim);                        // Subsequent calls to strtok() must pass a NULL pointer as the first argument, in order to get the next token in the string
    }
    tokens[i] = NULL;                                           // Tokens is reset to null
    return tokens;                                              // The null is returned. Allowing the program to continue reading the next line
}

void main_loop(void)
{
    char **args;
    int status;                                                  // While status is 1, the loop will continue infinitely

    do                                                           // A do while loop is used as it executes once before it checks the status
    {
        printf("~ ");                                            // "~" is printed in by the shell to indicate the start of every line
        line = read_line();                                      // The user input is being read and stored in "line"
        args = split_line(line);                                 // Line is then parsed and split up by its arguments
        status = execute(args);                                  // If line is understood then it is carried out

        free(line);                                              // The memory allocated to by malloc and realloc is deallocated
        free(args);
    }
    while (status);                                              // The loop will continue indefinitely, until the "exit" command is entered
}

int main(int argc, char **argv)
{
    printf("*********************************************************************\n");
    printf("Welcome to MyShell, enter \"help\" for basic information\n\n");

    main_loop();                                                  // The main infinite loop is called, allowing the program to repeat after every user determined processing and execution of a line

    return EXIT_SUCCESS;
}
