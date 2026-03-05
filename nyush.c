#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#include "io_redirect.h"
#include "pipeline.h"

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

        // if (arg_count == 0) continue;

        /* M6/M7: I/O redirection && M8/M9: pipes */
        Pipeline pipeline;
        if (parse_pipeline(args, arg_count, &pipeline) != 0) {  // parse the segmented commands into pipe
            // -- use data in pipe from now on --
            continue;  // print the errors if any
        }

        /* Built-in commands cannot appear after a pipe */
        int skip = 0; 
        for (int i = 1; i < pipeline.num_cmds; i++) {
            if (strcmp(pipeline.cmds[i].argv[0], "cd")   == 0 || strcmp(pipeline.cmds[i].argv[0], "exit") == 0){
                fprintf(stderr, "Error: invalid command\n");
                skip = 1;
                break;
            }
        }
        // if (skip) continue;

        /* M5: built-in commands (no redirection) */
        /* Built-in: cd */
        if (strcmp(pipeline.cmds[0].argv[0], "cd") == 0) {
            if (pipeline.num_cmds != 1 || pipeline.cmds[0].argc != 2 || pipeline.redir.input_file != NULL || pipeline.redir.output_file != NULL) {
                fprintf(stderr, "Error: invalid command\n");
                continue;
            }
            if (chdir(pipeline.cmds[0].argv[1]) != 0) {
                fprintf(stderr, "Error: invalid directory\n");
            }
            continue;
        }

        /* Built-in: exit */
        if (strcmp(pipeline.cmds[0].argv[0], "exit") == 0) {
            if (pipeline.num_cmds != 1 || pipeline.cmds[0].argc != 1 || pipeline.redir.input_file != NULL || pipeline.redir.output_file != NULL) {
                fprintf(stderr, "Error: invalid command\n");
                continue;
            }
            break; // terminate the shell
        }

        /* Execute pipeline */
        int num_pipes = pipeline.num_cmds - 1;
        int pipefds[num_pipes][2];  // the array of file descriptors that will be updated by pipe() syscall
        pid_t pids[pipeline.num_cmds]; // the array storing pids of each forked child(for each command, we'll fork a child)

        // Create pipes between adjacent commands
        for (int i = 0; i < num_pipes; i++) {
            if (pipe(pipefds[i]) < 0) {  // pipe() syscall updates pipefds[]
                perror("Pipe failed");
                skip = 1;
                break;
            }
        }
        if (skip) continue;

        // Fork a child for each command in the pipeline
        for (int i = 0; i < pipeline.num_cmds; i++) {
            pids[i] = fork();
            if (pids[i] < 0) {
                perror("Fork failed");
                break;
            }
            if (pids[i] == 0) {
                // Child: connect pipes
                if (i >= 1){
                    dup2(pipefds[i - 1][0], STDIN_FILENO);  // connecting the read end
                }
                if (i < num_pipes){
                    dup2(pipefds[i][1], STDOUT_FILENO);  // connecting the write end
                }

                // Close all pipe file descriptors in child
                for (int j = 0; j < num_pipes; j++) {
                    close(pipefds[j][0]);
                    close(pipefds[j][1]);
                }

                // Apply file redirection (< for first, > / >> for last)
                Redirection child_redir = {NULL, NULL, 0};
                if (i == 0){
                    child_redir.input_file = pipeline.redir.input_file;
                }
                if (i == pipeline.num_cmds - 1) {
                    child_redir.output_file = pipeline.redir.output_file;
                    child_redir.append = pipeline.redir.append;
                }
                if(apply_redirection(&child_redir) != 0)
                    exit(1);  // prints "Error: invalid file" on open failure


                // Locate program
                char exec_path[MAX_INPUT_SIZE];
                if (strchr(pipeline.cmds[i].argv[0], '/') != NULL) {  // a path is provided
                    strncpy(exec_path, pipeline.cmds[i].argv[0], sizeof(exec_path) - 1);
                    exec_path[sizeof(exec_path) - 1] = '\0';
                } else {   // no path provided
                    snprintf(exec_path, sizeof(exec_path), "/usr/bin/%s", pipeline.cmds[i].argv[0]);
                }

                // for each child, exec the final parsed argument: pipeline.cmds[i].argv
                execv(exec_path, pipeline.cmds[i].argv);
                fprintf(stderr, "Error: invalid program\n");
                exit(1);
            }
        }

        // Parent: close all pipe fds
        for (int j = 0; j < num_pipes; j++) {
            close(pipefds[j][0]);
            close(pipefds[j][1]);
        }

        // Wait for all children
        for (int i = 0; i < pipeline.num_cmds; i++) {
            int status;
            waitpid(pids[i], &status, 0);
        }

    }
    free(line);


    return 0;
}
