#ifndef JOBS_H
#define JOBS_H

#include <sys/types.h>

#define MAX_JOBS 100
#define MAX_CMD_LEN 2000

typedef struct {
    pid_t pid;
    char command[MAX_CMD_LEN];
} Job;

extern Job job_list[MAX_JOBS];
extern int job_count;

void add_job(pid_t pid, const char *command);
void remove_job(int index);
void print_jobs(void);

#endif
