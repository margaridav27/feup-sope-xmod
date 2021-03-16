#include "../include/signals.h"
#include "../include/utils.h"
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/wait.h>

// Parent flags
static volatile sig_atomic_t prompt = false; // Whether the user should be prompted

// Child flags
static volatile sig_atomic_t hold = false; // Whether the process should hold
static volatile sig_atomic_t resume = false; // Whether the process should resume
static volatile sig_atomic_t terminate = false; // Whether the process should terminate

void parent_sigint_handler(int signo) { prompt = true; }

void child_sigint_handler(int signo) { hold = true; }

void child_sigcont_handler(int signo) { resume = true; }

void child_sigterm_handler(int signo) { terminate = true; }

int setUpSignals(void) {
    // COMBACK: Change to sigaction
    // COMBACK: Stop using SIGTERM
    if (isParentProcess()) {
        signal(SIGINT, parent_sigint_handler);
        signal(SIGTERM, SIG_IGN);
    } else {
        signal(SIGINT, child_sigint_handler);
        signal(SIGCONT, child_sigcont_handler);
        signal(SIGTERM, child_sigterm_handler);
    }
    return 0;
}

void terminateProgramParent(void) {
    //COMBACK: LOG
    killpg(0, SIGCONT);
    killpg(0, SIGTERM);
    leave(1);
}

void continueProgramParent(void) {
    //COMBACK: LOG
    killpg(0, SIGCONT);
}

bool checkParentAction(void) {
    //COMBACK: LOG
    if (!prompt) return false;
    unsigned char answer;
    do {
        puts("Are you sure that you want to terminate? (Y/N)");
        answer = toupper(getchar());
        getchar(); // Discard newline
    } while (answer != 'Y' && answer != 'N');
    prompt = false;
    if (answer == 'N') {
        continueProgramParent();
        return false;
    }
    return true;
}

void childHoldAction(void) {
    // COMBACK: LOG
    raise(SIGSTOP);
    hold = false;
}

void childResumeAction(void) {
    // COMBACK: LOG
    resume = false;
}

void childTerminateAction(void) {
    // COMBACK: LOG
    terminate = false;
}

bool checkChildAction(void) {
    if (hold) childHoldAction();
    if (resume) childResumeAction();
    if (terminate) {
        childTerminateAction();
        return true;
    }
    return false;
}

bool checkTerminateSignal(void) {
    return isParentProcess() ? checkParentAction() : checkChildAction();
}