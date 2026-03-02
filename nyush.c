#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(){
    const int MAX_INPUT_SIZE = 1024;
    char cwd[MAX_INPUT_SIZE];

    // Get the current working directory
    if(getcwd(cwd, sizeof(cwd))!= NULL){

        // Get the basename from cwd
        char *basename;

        if (strcmp(cwd,"/") ==0){ // we're in the root directory
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
    }

    printf("\n");

    return 0;

    
}