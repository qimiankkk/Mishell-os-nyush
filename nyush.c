#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#include "io_redirect.h"

#define MAX_ARGS 1000

int main(void){
    const int MAX_INPUT_SIZE = 1024;
    char cwd[MAX_INPUT_SIZE];
    char *line = NULL;
    size_t len = 0;
    ssize_t read = 0;

    while(1){
        
        /* M1: Print the prompt */
        // Get the current working directory
        if(getcwd(cwd, sizeof(cwd))!= NULL){

            // Get the basename from cwd
            char *basename;
            if (strcmp(cwd,"/") ==0){ // root directory
                basename = "/";
            }else{
                basename = strrchr(cwd, '/'); // note: strrchr() returns a pointer to the last occurrence of '/'
                basename++; // skip the "/"
            }

            // Prompt printing 
            printf("[nyush %s]$ ", basename);
            fflush(stdout);
        }else{
            perror("Error getting current working directory.");
            exit(1);
        }

        /* M2: Read user input */
        read = getline(&line, &len, stdin);
        if (read == -1) break;  // captures EOF
        
        if(read > 0 && line[read-1]=='\n'){  // replace the '\n' char by '\0'
            line[read-1] = '\0';
        }

        if(strlen(line)==0){
            continue;
        }

        // printf("you entered: '%s' \n", line);

        /* M4: Parse multiple arguments (string tokenization)*/
        char *args[MAX_ARGS];
        int arg_count = 0;
        char *saveptr; // the save pointer for saving the context of tokens
        
        char *cmd = strtok_r(line, " ", &saveptr);
        while(cmd != NULL && arg_count < MAX_ARGS){
            args[arg_count++] = cmd;
            // arg_count++;
            cmd = strtok_r(NULL, " ", &saveptr);
        }
        args[arg_count] = NULL;

        if (arg_count == 0) continue;

        /* M6/M7: I/O redirection */
        Redirection redir;
        if (parse_redirection(args, &arg_count, &redir) != 0) {
            continue;  // print the parse error
        }

        /* M5: built-in commands */
        /* Built-in: cd */
        if(strcmp(args[0], "cd") == 0){
            if (arg_count != 2 ||
                redir.input_file != NULL || redir.output_file != NULL){
                fprintf(stderr, "Error: invalid command\n");
                continue;
            }
            if(chdir(args[1]) != 0){
                fprintf(stderr, "Error: invalid directory\n");
            }
            continue;
        }

        /* Built-in: exit */
        if(strcmp(args[0], "exit") == 0){
            if(arg_count != 1 ||
               redir.input_file != NULL || redir.output_file != NULL){
                fprintf(stderr, "Error: invalid command\n");
                continue;
            }
            break; // terminate the shell
        }


        // Locating programs
        char exec_path[MAX_INPUT_SIZE];
        if(strchr(args[0],'/')!= NULL){
            // a path is provided
            strncpy(exec_path, args[0], sizeof(exec_path) - 1);
            exec_path[sizeof(exec_path) - 1] = '\0';
        }else{
            // only a base name is provided=

            // strcpy(exec_path, "/usr/bin/");
            // strcat(exec_path, args[0]);

            // strlcpy(exec_path, "/usr/bin/", sizeof(exec_path));
            // strlcat(exec_path, args[0], sizeof(exec_path));

            snprintf(exec_path, sizeof(exec_path), "/usr/bin/%s", args[0]);
        }
        

        pid_t pid = fork();
        if (pid < 0) perror("Fork failed.");
        else if(pid == 0) {
            // child process
            if (apply_redirection(&redir) != 0) {
                exit(1);
            }
            execv(exec_path, args);
            fprintf(stderr, "Error: invalid program\n");
            exit(1);
        } else{
            // shell process
            int status;
            waitpid(pid, &status, 0);
        }

    }
    free(line);


    return 0;
}
