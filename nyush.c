#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main(){
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
            printf("[nyush %s]$", basename);
            fflush(stdout);
        }else{
            perror("Error getting current working directory.");
            exit(1);
        }

        /* M2: Read user input */
        read = getline(&line, &len, stdin);
        if (read == -1) break;  // captures EOF
        
        if(read > 0 && line[read-1]=='\n'){  // replace the '\n' char
            line[read-1] = '\0';
        }

        if(strlen(line)==0){
            continue;
        }

        // printf("you entered: '%s' \n", line);

        /* M3: Run simple programs */
        char *savecmd;
        // first command only
        char *cmd = strtok_r(line, " ", &savecmd);
        if (cmd == NULL) continue;

        // handles slash and searches for /usesr/bin
        char exec_path[1024];
        if(strchr(cmd,'/')!= NULL){
            // a path is provided
            strncpy(exec_path, cmd, sizeof(exec_path-1));
            exec_path[sizeof(exec_path) - 1] = '\0';
        }else{
            // only a base name is provided, search for /usr/bin
            snprintf(exec_path, sizeof(exec_path), "/usr/bin/%s", cmd);
        }
        char *args[] = {cmd, NULL}; // a NULL-terminated array of args for exec

        pid_t pid = fork();
        if(pid < 0) perror("Fork failed.");
        else if(pid == 0) {
            // child process
            execv(exec_path, args);
            perror("Error: invalid program");
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