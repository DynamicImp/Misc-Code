
#include "smallsh.h"

// Global variables
volatile pid_t foreground_pgid = -1; // Tracks the current foreground process group
struct job jobs[MAX_JOBS];
int job_count = 0; // Number of jobs running or stopped

// Handle Ctrl-C
void handle_sigint(int sig) {
    if (foreground_pgid > 0) {
        killpg(foreground_pgid, SIGINT); // Kill the foreground process group
    }
    printf("\nCommand> "); // Show the prompt again
    fflush(stdout);
}

// Handle Ctrl-Z
void handle_sigtstp(int sig) {
    if (foreground_pgid > 0) {
        killpg(foreground_pgid, SIGTSTP); // Stop the foreground process group
    }
    printf("\nCommand> "); // Show the prompt again
    fflush(stdout);
}

// Add a new job to the list
void add_job(pid_t pgid, char *cmd) {
    jobs[job_count].job_id = job_count + 1;
    jobs[job_count].pgid = pgid;
    strncpy(jobs[job_count].command, cmd, MAX_COMMAND_LEN);
    jobs[job_count].is_running = 1; // Job is running
    job_count++;
}

// Print all current jobs
void print_jobs() {
    for (int i = 0; i < job_count; i++) {
        printf("[%d] %s %s\n", jobs[i].job_id,
               jobs[i].is_running ? "Running" : "Stopped",
               jobs[i].command);
    }
}

// Bring a job to the foreground
void bring_to_foreground(int job_id) {
    if (job_id <= 0 || job_id > job_count) {
        printf("Invalid job ID\n");
        return;
    }
    pid_t pgid = jobs[job_id - 1].pgid;
    tcsetpgrp(STDIN_FILENO, pgid); // Give terminal control to the job
    killpg(pgid, SIGCONT); // Continue the process

    foreground_pgid = pgid; // Track the foreground job
    int status;
    waitpid(-pgid, &status, WUNTRACED); // Wait for it to finish or stop
    tcsetpgrp(STDIN_FILENO, getpgrp()); // Give terminal back to the shell
    foreground_pgid = -1;
}

// Resume a job in the background
void resume_in_background(int job_id) {
    if (job_id <= 0 || job_id > job_count) {
        printf("Invalid job ID\n");
        return;
    }
    pid_t pgid = jobs[job_id - 1].pgid;
    killpg(pgid, SIGCONT); // Continue the process
    jobs[job_id - 1].is_running = 1; // Mark it as running
    printf("Job [%d] %s resumed in background\n", job_id, jobs[job_id - 1].command);
}

// Kill a job
void terminate_job(int job_id) {
    if (job_id <= 0 || job_id > job_count) {
        printf("Invalid job ID\n");
        return;
    }
    pid_t pgid = jobs[job_id - 1].pgid;
    killpg(pgid, SIGKILL); // Kill the process group
    printf("Job [%d] %s terminated\n", job_id, jobs[job_id - 1].command);
}

// Main program
int main() {
    // Set up signal handlers for Ctrl-C and Ctrl-Z
    struct sigaction sa_int, sa_tstp;
    sa_int.sa_handler = handle_sigint;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = SA_RESTART;

    sa_tstp.sa_handler = handle_sigtstp;
    sigemptyset(&sa_tstp.sa_mask);
    sa_tstp.sa_flags = SA_RESTART;

    sigaction(SIGINT, &sa_int, NULL);
    sigaction(SIGTSTP, &sa_tstp, NULL);

    char command[MAX_COMMAND_LEN];
    while (1) {
        printf("Command> "); // Print the shell prompt
        fflush(stdout);

        if (fgets(command, MAX_COMMAND_LEN, stdin) == NULL) {
            break; // Exit if EOF is reached
        }

        // Strip newline from command
        command[strcspn(command, "\n")] = 0; 

        // Skip empty input
        if (strlen(command) == 0) continue;

        // Split command into arguments
        char *args[MAX_COMMAND_LEN / 2 + 1]; // Array for arguments
        int arg_count = 0;
        char *token = strtok(command, " ");
        while (token != NULL) {
            args[arg_count++] = token;
            token = strtok(NULL, " ");
        }
        args[arg_count] = NULL; // Null-terminate the arguments array

        // Built-in commands
        if (strcmp(args[0], "jobs") == 0) {
            print_jobs();
        } else if (strcmp(args[0], "fg") == 0) {
            if (arg_count < 2) {
                printf("Usage: fg <job_id>\n");
            } else {
                int job_id = atoi(args[1]);
                bring_to_foreground(job_id);
            }
        } else if (strcmp(args[0], "bg") == 0) {
            if (arg_count < 2) {
                printf("Usage: bg <job_id>\n");
            } else {
                int job_id = atoi(args[1]);
                resume_in_background(job_id);
            }
        } else if (strcmp(args[0], "kill") == 0) {
            if (arg_count < 2) {
                printf("Usage: kill <job_id>\n");
            } else {
                int job_id = atoi(args[1]);
                terminate_job(job_id);
            }
        } else {
            // Handle external commands
            pid_t pid = fork();
            if (pid == 0) {
                // Child process
                setpgid(0, 0); // Create a new process group
                execvp(args[0], args);
                perror("Command execution failed");
                exit(1);
            } else if (pid > 0) {
                // Parent process
                setpgid(pid, pid); // Set child's process group
                if (arg_count > 1 && strcmp(args[arg_count - 1], "&") == 0) {
                    // Background job
                    args[arg_count - 1] = NULL; // Remove "&"
                    printf("Started background job [%d] %s\n", job_count + 1, args[0]);
                    add_job(pid, args[0]); // Add job to list
                } else {
                    // Foreground job
                    tcsetpgrp(STDIN_FILENO, pid); // Give control to the process
                    foreground_pgid = pid;

                    int status;
                    waitpid(pid, &status, WUNTRACED); // Wait for it to finish or stop
                    tcsetpgrp(STDIN_FILENO, getpgrp()); // Give control back to the shell
                    foreground_pgid = -1;
                }
            } else {
                perror("Fork failed");
            }
        }
    }

    return 0;
}
