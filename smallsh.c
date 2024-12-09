#include "smallsh.h"

// Global variables
Job jobs[MAX_JOBS];
int job_count = 0;

void init_shell() {
    signal(SIGINT, sigint_handler);  // Handle Ctrl-C
    signal(SIGTSTP, sigtstp_handler); // Handle Ctrl-Z
    printf("Welcome to smallsh!\n");
}

void sigint_handler(int signo) {
    pid_t fg_pgid = tcgetpgrp(STDIN_FILENO);
    if (fg_pgid != getpid()) {
        kill(-fg_pgid, SIGINT);
    }
    printf("\nCommand> ");
    fflush(stdout);
}

void sigtstp_handler(int signo) {
    pid_t fg_pgid = tcgetpgrp(STDIN_FILENO);
    if (fg_pgid != getpid()) {
        kill(-fg_pgid, SIGTSTP);
    }
    printf("\nCommand> ");
    fflush(stdout);
}

void add_job(pid_t pgid, const char *command) {
    if (job_count >= MAX_JOBS) {
        printf("Job list full!\n");
        return;
    }
    jobs[job_count].job_id = job_count + 1;
    jobs[job_count].pgid = pgid;
    strcpy(jobs[job_count].command, command);
    jobs[job_count].status = 0; // Running
    job_count++;
}

void remove_job(int job_id) {
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].job_id == job_id) {
            for (int j = i; j < job_count - 1; j++) {
                jobs[j] = jobs[j + 1];
            }
            job_count--;
            return;
        }
    }
    printf("Job not found!\n");
}

void list_jobs() {
    for (int i = 0; i < job_count; i++) {
        printf("[%d] %s (%s)\n", jobs[i].job_id, jobs[i].command,
               jobs[i].status == 0 ? "Running" : jobs[i].status == 1 ? "Stopped" : "Terminated");
    }
}

void bg_job(int job_id) {
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].job_id == job_id && jobs[i].status == 1) {
            kill(-jobs[i].pgid, SIGCONT);
            jobs[i].status = 0; // Running
            return;
        }
    }
    printf("Job not found or not stopped!\n");
}

void fg_job(int job_id) {
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].job_id == job_id) {
            tcsetpgrp(STDIN_FILENO, jobs[i].pgid);
            kill(-jobs[i].pgid, SIGCONT);
            int status;
            waitpid(-jobs[i].pgid, &status, WUNTRACED);
            tcsetpgrp(STDIN_FILENO, getpid());
            if (WIFSTOPPED(status)) {
                jobs[i].status = 1; // Stopped
            } else {
                remove_job(job_id); // Completed or terminated
            }
            return;
        }
    }
    printf("Job not found!\n");
}

int main() {
    char input[MAXBUF];
    pid_t pid;
    init_shell();

    while (1) {
        printf("Command> ");
        fflush(stdout);
        if (fgets(input, MAXBUF, stdin) == NULL) {
            printf("\n");
            break;
        }

        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "exit") == 0) {
            printf("Exiting smallsh...\n");
            break;
        } else if (strcmp(input, "jobs") == 0) {
            list_jobs();
        } else if (strncmp(input, "bg ", 3) == 0) {
            bg_job(atoi(input + 3));
        } else if (strncmp(input, "fg ", 3) == 0) {
            fg_job(atoi(input + 3));
        } else {
            pid = fork();
            if (pid == 0) {
                setpgid(0, 0);
                execlp(input, input, (char *)NULL);
                perror("Exec failed");
                exit(1);
            } else if (pid > 0) {
                setpgid(pid, pid);
                if (strchr(input, '&') == NULL) {
                    tcsetpgrp(STDIN_FILENO, pid);
                    int status;
                    waitpid(pid, &status, WUNTRACED);
                    tcsetpgrp(STDIN_FILENO, getpid());
                    if (WIFSTOPPED(status)) {
                        add_job(pid, input);
                    }
                } else {
                    add_job(pid, input);
                    printf("Background job started: %d\n", pid);
                }
            } else {
                perror("Fork failed");
            }
        }
    }

    return 0;
}
