#include "../include/signals.h"
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/wait.h>
#include <log.h>
#include <sys/resource.h>
#include <unistd.h>
#include <string.h>
#include "../include/time_ctrl.h"
#include "../include/utils.h"
#include "../include/log.h"

extern int number_of_files, number_of_modified_files;
const char *path;
const int *const no_files = &number_of_files;
const int *const no_modified_files = &number_of_modified_files;

void generic_signal_handler(int signo) {
    log_signal_received(signo);

    if (signo == SIGINT) isParentProcess() ? parent_sigint_handler(signo) : child_sigint_handler(signo);
    else if (signo == SIGTERM && !isParentProcess()) leave(1);
    else {
        // Repeat the requested action
        signal(signo, SIG_DFL);
        raise(signo);
    }
}

void parent_sigint_handler(int signo) {
    // log_signal_received(SIGINT);
    // COMBACK: Find a better solution
    usleep(5000); // Wait for other prompts
    log_current_status(path, *no_files, *no_modified_files);
    char buffer[10];
    do {
        const char *prompt = "Are you sure that you want to terminate? (Y/N) ";
        write(STDOUT_FILENO, prompt, strlen(prompt));
        int n = read(STDIN_FILENO, buffer, 10);
        buffer[n] = '\0';
    } while (buffer[0] != 'Y' && buffer[0] != 'y' && buffer[0] != 'N' && buffer[0] != 'n');
    if (buffer[0] == 'Y' || buffer[0] == 'y') terminateProgramParent();
    else if (buffer[0] == 'N' || buffer[0] == 'n') continueProgramParent();
    // perror("");
}

void child_sigint_handler(int signo) {
    log_current_status(path, *no_files, *no_modified_files);

    //COMBACK: Verify if this allows us to wait for children. If not, handle sigcont and convert to pause?
    log_signal_sent(SIGSTOP, getpid());
    log_signal_received(SIGSTOP);
    raise(SIGSTOP);
}

int setUpSignals(const char *p) {
    path = p;
    // COMBACK: Change to sigaction
    // COMBACK: Stop using SIGTERM, convert to SIGUSR
    for (int signo = 1; signo < SIGRTMIN; ++signo) {
        if (signo == SIGKILL || signo == SIGSTOP || signo == SIGCHLD) continue;
        signal(signo, generic_signal_handler);
    }
    if (isParentProcess()) signal(SIGTERM, SIG_IGN);
    return 0;
}

void terminateProgramParent(void) {
    killpg(0, SIGCONT); // Wake up children
    log_signal_sent(SIGCONT, getpgrp());
    killpg(0, SIGTERM); // Ask children to terminate
    log_signal_sent(SIGTERM, getpgrp());
    leave(1); // Abnormal termination
}

void continueProgramParent(void) {
    killpg(0, SIGCONT); // Wake up children
    log_signal_sent(SIGCONT, getpgrp());
}