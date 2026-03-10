#include <stdio.h>
#include <string.h>
#include "jobs.h"

Job job_list[MAX_JOBS];
int job_count = 0;

void add_job(pid_t pid, const char *command) {
    if (job_count >= MAX_JOBS) return;
    job_list[job_count].pid = pid;
    strncpy(job_list[job_count].command, command, MAX_CMD_LEN - 1);
    job_list[job_count].command[MAX_CMD_LEN - 1] = '\0';
    job_count++;
}

void remove_job(int index) {
    for (int i = index; i < job_count - 1; i++) {
        job_list[i] = job_list[i + 1];
    }
    job_count--;
}

void print_jobs(void) {
    for (int i = 0; i < job_count; i++) {
        printf("[%d] %s\n", i + 1, job_list[i].command);
    }
}
