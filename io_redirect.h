#ifndef IO_REDIRECT_H
#define IO_REDIRECT_H

typedef struct {
    char *input_file;  //filename for stdin redirection(<)      
    char *output_file;  //filename for stdout redirection (> or >>)
    int append;  //1 for >> (append), 0 for > (truncate)      
} Redirection;

/* 
This function implements the following:
1. scans through the args array sequentially, and extracts the indx of token <, >, >>.
2. validate the redirection section and parse the i/o redirected filename to the data field in Redirection.
3. store command args to args array.
4. prints "Error: invalid command" on any parse error.
5. returns -1 when fails. 
*/
int parse_redirection(char **args, int *arg_count, Redirection *redir);

/*
This function implements the following:
1. try to open input redirected file, with O_RDONLY. And implements the redirection using dup2.
2. try to open output redirected file, with O_WRONLY|O_CREAT, as wellas 0_TRUNC for > and 0_APPEND for >>. And implements the redirection using dup2.
3. prints "Error: invalid file" on open failure.
4. returns -1 when fails.
*/
int apply_redirection(const Redirection *redir);

#endif
