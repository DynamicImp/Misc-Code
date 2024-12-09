#ifndef SMALLSH_H
#define SMALLSH_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAXBUF 1024
#define MAX_JOBS 100

typedef struct {
    int job_id;
    pid_t pgid;
    char command[MAXBUF];
    int status; // 0: running, 1: stopped, 2: terminated
} Job;

// Function prototypes
void init_shell();
void sigint_handler(int signo);
void sigtstp_handler(int signo);
void add_job(pid_t pgid, const char *command);
void remove_job(int job_id);
void list_jobs();
void bg_job(int job_id);
void fg_job(int job_id);

#endif
