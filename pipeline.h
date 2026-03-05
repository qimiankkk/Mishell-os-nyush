#ifndef PIPELINE_H
#define PIPELINE_H

#include "io_redirect.h"


#define MAX_PIPELINE_CMDS 1000

typedef struct {
    char **argv;  // points to the original args array, NULL-terminated
    int argc;
} CmdSeg;

typedef struct {
    CmdSeg cmds[MAX_PIPELINE_CMDS];
    int num_cmds;
    Redirection redir;  /* < for first cmd, > / >> for last cmd */
} Pipeline;

/*
This function splits args by '|' into pipeline segments, then parses redirection
for each segment (< only valid on first cmd, > / >> only on last cmd).
Returns 0 on success, -1 on error (message printed to stderr).
*/
int parse_pipeline(char **args, int arg_count, Pipeline *pl);

#endif
