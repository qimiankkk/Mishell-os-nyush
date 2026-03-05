#include "io_redirect.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* Characters forbidden in [cmdname], [arg], and [filename] per the grammar. */
static int is_invalid_char(char c){
    return c == ' '|| c == '\t'|| c == '>' || c == '<' || c == '|'|| c == '*'|| c == '!' || c == '`'|| c == '\'' || c == '"';
}

/* Return 1 if the string `s` is a valid filename(non-empty, no invalid chars). */
static int is_valid_filename(const char *s){
    if (s == NULL || s[0] == '\0')
        return 0;
    for (int i = 0; s[i]; i++) {
        if (is_invalid_char(s[i]))
            return 0;
    }
    return 1;
}

/* 
This function implements the following:
1. scans through the args array sequentially, and extracts the indx of token <, >, >>.
2. validate the redirection section and parse the i/o redirected filename to the data field in Redirection.
3. store command args to args array.
4. prints "Error: invalid command" on any parse error.
5. returns -1 when fails. 
*/
int parse_redirection(char **args, int *arg_count, Redirection *redir){
    redir->input_file = NULL;
    redir->output_file = NULL;
    redir->append = 0;

    /* Locate the first redirection operator */
    int cmd_end = *arg_count; // index of redir operator
    for (int i = 0; i < *arg_count; i++) {
        if (strcmp(args[i], "<")  == 0 ||
            strcmp(args[i], ">")  == 0 ||
            strcmp(args[i], ">>") == 0) {
            cmd_end = i;
            break;
        }
    }

    /* Parse the redirection section (everything from cmd_end onward). */
    int i = cmd_end;
    while (i < *arg_count) {
        if (strcmp(args[i], "<") == 0) {
            if (redir->input_file != NULL){ 
                fprintf(stderr, "Error: invalid command\n");
                return -1;
            }
            i++; // the starting index of input filename
            if(i >= *arg_count || !is_valid_filename(args[i])){
                fprintf(stderr, "Error: invalid command\n");
                return -1;
            }
            redir->input_file = args[i];
            i++;
        } else if(strcmp(args[i], ">>") == 0 || strcmp(args[i], ">") == 0) {
            if (redir->output_file != NULL){
                fprintf(stderr, "Error: invalid command\n");
                return -1;
            }
            int is_append = (strcmp(args[i], ">>") == 0);
            i++; // the starting index of the output file
            if (i >= *arg_count || !is_valid_filename(args[i])) {
                fprintf(stderr, "Error: invalid command\n");
                return -1;
            }
            redir->output_file = args[i];
            redir->append = is_append;
            i++;
        } else {
            // a non-redirection token in the redirection section
            fprintf(stderr, "Error: invalid command\n");
            return -1;
        }
    }

    // truncate args to only store command args
    *arg_count = cmd_end;
    args[cmd_end] = NULL;

    return 0;
}


/*
This function implements the following:
1. try to open input redirected file, with O_RDONLY. And implements the redirection using dup2.
2. try to open output redirected file, with O_WRONLY|O_CREAT, as well as O_TRUNC for > and O_APPEND for >>. And implements the redirection using dup2.
3. prints "Error: invalid file" on open failure.
4. returns -1 when fails.
*/
int apply_redirection(const Redirection *redir)
{
    if (redir->input_file != NULL) {
        int fd = open(redir->input_file, O_RDONLY); // open the given redirected input file w/ read only 
        if (fd < 0) {
            fprintf(stderr, "Error: invalid file\n");
            return -1;
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }

    if (redir->output_file != NULL) {
        int flags = O_WRONLY | O_CREAT;
        if (redir->append) // >>
            flags |= O_APPEND;
        else   // >
            flags |= O_TRUNC;

        int fd = open(redir->output_file, flags, 0644);
        if (fd < 0) {
            fprintf(stderr, "Error: invalid file\n");
            return -1;
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }

    return 0;
}
