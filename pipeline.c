#include "pipeline.h"

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


/*
This function splits args by '|' into pipeline segments, then parses redirection
for each segment (< only valid on first cmd, > / >> only on last cmd).
Returns 0 on success, -1 on error (message printed to stderr).
*/
int parse_pipeline(char **args, int arg_count, Pipeline *pipe){
    pipe->num_cmds = 0;
    pipe->redir.input_file  = NULL;
    pipe->redir.output_file = NULL;
    pipe->redir.append      = 0;

    // split the args array by '|' into segments.
    int start = 0;
    for (int i = 0; i <= arg_count; i++) {
        if (i == arg_count || strcmp(args[i], "|") == 0) {
        // if (strcmp(args[i], "|" ) == 0) {
            if (i == start) {
                // empty segment: leading |, trailing |, or ||
                fprintf(stderr, "Error: invalid command\n");
                return -1;
            }
            if (pipe->num_cmds >= MAX_PIPELINE_CMDS) {
                fprintf(stderr, "Error: invalid command\n");
                return -1;
            }
            pipe->cmds[pipe->num_cmds].argv = &args[start];  // store each command separately into an array of datafield CmdSeg
            pipe->cmds[pipe->num_cmds].argc = i - start;  // store the length of the segmented command
            pipe->num_cmds++;
            if (i < arg_count)
                args[i] = NULL;   //replace | with NULL
            start = i + 1;
        }
        /* if (i == arg_count){
            pipe->cmds[0].argv = &args;
            pipe->cmds[0].argc = arg_count;
            pipe->num_cmds = 1;
        } */
    }

    if (pipe->num_cmds == 0) {
        fprintf(stderr, "Error: invalid command\n");
        return -1;
    }

    // parse redirection for each segment.
    for (int s = 0; s < pipe->num_cmds; s++) {
        CmdSeg *seg = &pipe->cmds[s];  // seg is the s-th cmd in the pipeline
        Redirection seg_redir;  // the temp redir
        if (parse_redirection(seg->argv, &seg->argc, &seg_redir) != 0){  // vlidate seg; parse filenames to the datafiled of seg_redir
            return -1;
        }  

        if (seg_redir.input_file != NULL) {
            if (s != 0) {  // < is only valid on the first command
                fprintf(stderr, "Error: invalid command\n");
                return -1;
            }
            pipe->redir.input_file = seg_redir.input_file;
        }

        if (seg_redir.output_file != NULL) {
            if (s != pipe->num_cmds - 1) {  // > or >> is only valid on the last command
                fprintf(stderr, "Error: invalid command\n");
                return -1;
            }
            pipe->redir.output_file = seg_redir.output_file;
            pipe->redir.append = seg_redir.append;
        }

        // each segment must still have a command name after stripping redirection
        if (seg->argc == 0) {
            fprintf(stderr, "Error: invalid command\n");
            return -1;
        }

        // NULL-terminate for execv
        seg->argv[seg->argc] = NULL;
    }

    return 0;
}
