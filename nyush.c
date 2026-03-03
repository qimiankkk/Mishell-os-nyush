#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
        if (read == -1){  // captures EOF
            break;
        }
        if(read > 0 && line[read-1]=='\n'){  // replace the '\n' char
            line[read-1] = '\0';
        }

        if(strlen(line)==0){
            continue;
        }

        printf("you entered: '%s' \n", line);

    }
    free(line);


    return 0;
}