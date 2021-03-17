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

// Parent flags
static volatile sig_atomic_t prompt = false; // Whether the user should be prompted

void generic_signal_handler(int signo) {
    log_signal_received(signo);

    // Repeat the requested action
    signal(signo, SIG_DFL);
    raise(signo);

    //COMBACK: Handle signals here

    // COMBACK: Allow future logging
    // signal(signo, generic_signal_handler);
}

void parent_sigint_handler(int signo) {
    log_signal_received(SIGINT);
    // int n = 0;
    // siginfo_t s;
    // while ((n = waitid(P_ALL, 0, &s, WNOWAIT | WUNTRACED)) >= 0) {
    //     printf("%d vs %d\n", n, number_of_children);
    //     ++n;
    // }
    usleep(5000);
    log_current_status(path, *no_files, *no_modified_files);

    // COMBACK: Question here makes the question appear all of the time? Is this good?
    // if (prompt) return;
    // prompt = true;
    unsigned char answer;
    do {
        puts("Are you sure that you want to terminate? (Y/N)");
        answer = toupper(getchar());
        getchar(); // Discard newline
    } while (answer != 'Y' && answer != 'N');
    if (answer == 'Y') terminateProgramParent();
    else continueProgramParent();
}

void child_sigint_handler(int signo) {
    log_signal_received(signo);
    log_current_status(path, *no_files, *no_modified_files);

    // COMBACK: this vs pause
    log_signal_sent(SIGSTOP, getpid());
    log_signal_received(SIGSTOP);
    raise(SIGSTOP);
}

void child_sigcont_handler(int signo) {
    log_signal_received(signo);
    // resume = true;
}

void child_sigterm_handler(int signo) {
    // COMBACK: Could we force the file descriptors to be closed?
    log_signal_received(signo);
    leave(1);
}

int setUpSignals(const char *p) {
    path = p;
    // COMBACK: Change to sigaction
    // COMBACK: Stop using SIGTERM
    // COMBACK: After switch, log these signals to the generic handler
    for (int signo = 1; signo < SIGRTMIN; ++signo) {
        if (signo == SIGKILL || signo == SIGSTOP || signo == SIGCHLD) continue;
        signal(signo, generic_signal_handler);
    }
    if (isParentProcess()) {
        signal(SIGINT, parent_sigint_handler);
        signal(SIGTERM, SIG_IGN);
    } else {
        signal(SIGINT, child_sigint_handler);
        signal(SIGCONT, child_sigcont_handler);
        signal(SIGTERM, child_sigterm_handler);
    }
    return 0;
    // COMBACK: Log remaining signals to generic handler
}

void terminateProgramParent(void) {
    killpg(0, SIGCONT);
    killpg(0, SIGTERM);
    leave(1);
}

void continueProgramParent(void) {
    log_signal_sent(SIGCONT, getpgrp());
    killpg(0, SIGCONT);
}