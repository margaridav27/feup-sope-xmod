//COMBACK: Explain header usages
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

const char *path;
extern int number_of_files, number_of_modified_files;
const int *const no_files = &number_of_files;
const int *const no_modified_files = &number_of_modified_files;

void generic_signal_handler(int sig_no) {
    //COMBACK: Look into error return value
    log_signal_received(sig_no);

    // Our own actions
    if (sig_no == SIGINT) {
        //COMBACK: Look into error return value
        isParentProcess() ? parent_sigint_handler(sig_no) : child_sigint_handler(sig_no);
    } else if (sig_no == SIGTERM && !isParentProcess()) {
        //COMBACK: Look into error return value
        leave(1);
    }
        // Repeat the requested actions
    else {
        //COMBACK: Look into error return value
        signal(sig_no, SIG_DFL);
        //COMBACK: Look into error return value
        raise(sig_no);
    }
}

void parent_sigint_handler(int sig_no) {
    // COMBACK: Find a better solution for syncronizing this
    //COMBACK: Look into error return value
    usleep(5000); // Wait for other prompts
    //COMBACK: Check signal safety
    //COMBACK: Look into error return value
    log_current_status(path, *no_files, *no_modified_files);
    char c = 'Y';
    do {
        const char *prompt = "Are you sure that you want to terminate? (Y/N) ";
        //COMBACK: Look into error return value
        write(STDOUT_FILENO, prompt, strlen(prompt));
        //COMBACK: Look into error return value
        fsync(STDOUT_FILENO);
        //COMBACK: Look into error return value
        int n = read(STDIN_FILENO, &c, 1);
        if (n == -1) break;
    } while (c != 'Y' && c != 'y' && c != 'N' && c != 'n');
    //COMBACK: Look into error return value
    if (c == 'Y' || c == 'y') terminateProgramParent();
        //COMBACK: Look into error return value
    else if (c == 'N' || c == 'n') continueProgramParent();
}

void child_sigint_handler(int sig_no) {
    //COMBACK: Check signal safety
    log_current_status(path, *no_files, *no_modified_files);
    //COMBACK: Look into error return value
    log_signal_sent(SIGSTOP, getpid());
    //COMBACK: Look into error return value
    log_signal_received(SIGSTOP);
    //COMBACK: Try to find a way to use this to alert the parent that we are done printing. If it is not possible, 
    // pause the thread instead
    //COMBACK: Look into error return value
    raise(SIGSTOP);
}

int setUpSignals(const char *p) {
    // COMBACK: Change signal setup to use sigaction
// COMBACK: Find a better way to forward this argument
    path = p;
    for (int sig_no = 1; sig_no < SIGRTMIN; ++sig_no) {
        if (sig_no == SIGKILL || sig_no == SIGSTOP || sig_no == SIGCHLD) continue;
        //COMBACK: Look into error return value
        signal(sig_no, generic_signal_handler);
    }
    // COMBACK: Stop using SIGTERM as the termination signal, convert to SIGUSR
    //COMBACK: Look into error return value
    if (isParentProcess()) signal(SIGTERM, SIG_IGN);
    return 0;
}

void terminateProgramParent(void) {
    //COMBACK: Look into error return value
    killpg(0, SIGCONT); // Wake up children
    //COMBACK: Look into error return value
    log_signal_sent(SIGCONT, getpgrp());
    //COMBACK: Look into error return value
    killpg(0, SIGTERM); // Ask children to terminate
    //COMBACK: Look into error return value
    log_signal_sent(SIGTERM, getpgrp());
    //COMBACK: Look into error return value
    leave(1); // Abnormal termination: exit code 1
}

void continueProgramParent(void) {
    //COMBACK: Look into error return value
    killpg(0, SIGCONT); // Wake up children
    //COMBACK: Look into error return value
    log_signal_sent(SIGCONT, getpgrp());
}
