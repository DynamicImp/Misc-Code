
#ifndef SMALLSH_H
#define SMALLSH_H

#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_COMMAND_LEN 1024
#define MAX_JOBS 100

// Job info
struct job {
    int job_id; // Job number
    pid_t pgid; // Process group ID
    char command[MAX_COMMAND_LEN]; // Command string
    int is_running; // 1 = running, 0 = stopped
};

// Shared variables
extern volatile pid_t foreground_pgid;
extern struct job jobs[MAX_JOBS];
extern int job_count;

// Functions we'll use
void handle_sigint(int sig);
void handle_sigtstp(int sig);
void add_job(pid_t pgid, char *cmd);
void print_jobs();
void bring_to_foreground(int job_id);
void resume_in_background(int job_id);
void terminate_job(int job_id);

#endif
