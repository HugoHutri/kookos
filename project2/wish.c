#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

/* Colors for the output */
#define YELLOW "\033[1;33m"
#define RESET "\033[0m"

/* Error types */
#define ERROR_DEFAULT "An error has occured\n"
#define ERROR_UNKNOWN_COMMAND "Error: Unknown command\n"
#define ERROR_TOO_MANY_ARGS "Error: Too many arguments\n"
#define ERROR_TOO_FEW_ARGS "Error: Too few arguments\n"
#define ERROR_INPUT_FILE "An Error with input file\n"
#define ERROR_MEMORY "Error: Memory allocation failed"

/* References
https://stackoverflow.com/questions/19099663/how-to-correctly-use-fork-exec-wait
https://brennan.io/2015/01/16/write-a-shell-in-c/
https://support.sas.com/documentation/onlinedoc/sasc/doc/lr2/waitpid.htm
*/


/* Linked list for paths */
typedef struct PATH {
    char* address;
    struct PATH *next;
} Path;

/* Handle error messages */
void shell_error(char* error_message) {
    write(STDERR_FILENO, error_message, strlen(error_message));
    /* Exit if there is an error with input file */
    if(!strcmp(error_message,ERROR_INPUT_FILE)) exit(1);
}

/* Read a line from user */
char* command_read(FILE* file) {
    char *line;
    size_t buffer_size = 0;
    getline(&line, &buffer_size, (file)?file:stdin);
    return line;
}

/* Launch the command with execv */
int command_launch(char** args, Path** path) {
    pid_t childID;
    int status;
    char addr[100];

    /* Loop every path */
    Path* ptr = *path; 
    while(ptr) {
        /* addr = address + command */
        strcpy(addr,"");
        strcat(addr, ptr->address);
        strcat(addr, "/");
        strcat(addr, args[0]);

        /* Check if the file exists and is executable */
        if(access(addr, X_OK)) {
            /* Not found */
            ptr = ptr->next;
            continue;
        }

        childID = fork();
        if(childID == 0) {
            /* Child prosess */

            execv(addr, args);
            exit(1);
        } else if (childID < 0) {
            shell_error(ERROR_UNKNOWN_COMMAND);
        } else {
            /* Parent */
            do {
                waitpid(childID, &status, WUNTRACED);
                if(WIFSTOPPED(status)) return 1;
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }
        return 1;
    }
    shell_error(ERROR_UNKNOWN_COMMAND);
    return 1;

}

/* Parse command parameters */
char** command_parse(char* line) {
    int buffer_size = 64;
    int pos = 0;
    char** args;
    char* arg;

    /* Malloc memory for the first argument */
    if(!(args = malloc(buffer_size * sizeof(char*)))) {
        shell_error(ERROR_MEMORY);
    }
    arg = strtok(line, " \t\r\n\a");
    while (arg) {
        /* Set argument to args-list */
        args[pos] = arg;
        pos++;

        /* Reallocate memory if the buffer is too small */
        if(pos >= buffer_size) {
            buffer_size += 64;
            if(!(args = realloc(args, buffer_size * sizeof(char*)))) {
                shell_error(ERROR_MEMORY);
            }
        }
        arg = strtok(NULL, " \t\r\n\a");
    }
    /* Set the last argument to NULL (= end of the args) */
    args[pos] = NULL;
    return args;
}

/* Buildin command for cd */
int cmd_cd(char** args) {
    if(!args[1]) shell_error(ERROR_TOO_FEW_ARGS);
    else chdir(args[1]);
    return 1;
}

/* Free memory from path and from address */
void free_path(Path* first) {
    Path* ptr = first;
    while(ptr) {
        first = ptr->next;
        free(ptr->address);
        free(ptr);
        ptr = first;
    }
}

/* Print all paths stored in linked list */
void print_path(Path* first) {
    Path* ptr = first;
    while(ptr) {
        printf("%s ",ptr->address);
        ptr = ptr->next;
    }
    printf("\n");
}

/* Buildin command for setting paths */
int cmd_path(char** args, Path **first) {
    char* addr;
    Path *new = NULL;
    Path *last = NULL;
    int pos = 0;

    if(args[1] == NULL) {
        /* Remove paths */
        free_path(*first);
        *first = NULL;
        return 1;
    }


    while(1) {
        pos++;
        /* Malloc memory for the string */
        if(!(addr = (char*) malloc(sizeof(args[pos])))) {
            shell_error(ERROR_MEMORY);
        }
        if(args[pos] == NULL) break;

        /* Malloc memory for the path node */
        if(!(new = (Path*) malloc(sizeof(Path)))) {
            shell_error(ERROR_MEMORY);
        }

        /* Assign values */
        strcpy(addr, args[pos]);
        new->address = addr;
        new->next = NULL;

        /* Linked list operations */
        if((*first) == NULL) {
            *first = new;
            last = *first;
            continue;
        }
        last = *first;
        while(last->next != NULL) {
            last = last->next;
        }
        last->next = new;
        last = new;
    }
    /* Print all paths from the linked list */
    print_path(*first);
    return 1;
}

int cmd_exit(char** args) {
    if(args[1] == NULL) return 0;
    shell_error(ERROR_TOO_MANY_ARGS);
    return 1;
}

/* Execute the command */
int command_execute(char** args, Path** path) {
    if(args[0] == NULL)             return 1;
    if(!strcmp(args[0], "exit"))    return cmd_exit(args);
    if(!strcmp(args[0], "cd"))      return cmd_cd(args);
    if(!strcmp(args[0], "path"))    return cmd_path(args, path);
    return command_launch(args, path);
}

Path* init_path() {
    /* Set default path to /bin */
    Path* new;
    char* addr;

    /* Malloc memory for the string */
    if(!(addr = (char*) malloc(8*sizeof(char)))) {
        shell_error(ERROR_MEMORY);
    }
    /* Malloc memory for the path node */
    if(!(new = (Path*) malloc(sizeof(Path)))) {
        shell_error(ERROR_MEMORY);
    }

    /* Assign values */
    strcpy(addr, "/bin");
    new->address = addr;
    new->next = NULL;

    return new;
}

/* Main loop */
void command_loop(FILE* input) {
    char* line;
    char** args;
    Path* path = init_path();
    int status;

    do {
        /* Print "wish> " if the output is terminal */
        if(!input) {
            printf(YELLOW);
            printf("wish>");
            printf(RESET);
            printf(" ");
        }


        /* Read, parse and run the command */
        line    = command_read(input);
        args    = command_parse(line);
        status  = command_execute(args, &path);

        /* Free the memory allocated by this cycle */
        free(line);
        free(args);
        
    } while (status); /* Exit if the returned status is 0 */
    
}

/* Main function */
int main(int argc, char** argv) {
    FILE* input = NULL;
    if(argc > 2) {
        shell_error(ERROR_INPUT_FILE);
        return 0;
    }
    if(argc == 2) {
        input = fopen(argv[1],"r");
        if(!input) shell_error(ERROR_INPUT_FILE);
    }

    command_loop(input);

    if(argc == 2) fclose(input);
    return 0;
}
 
/* 
example path command:
path /bin/ /usr/bin/
*/