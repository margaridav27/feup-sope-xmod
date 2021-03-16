#include "../include/signals.h"
#include "../include/utils.h"
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/wait.h>
#include <log.h>
#include <unistd.h>
#include <string.h>

extern int number_of_files, number_of_modified_files, number_of_children;

// Parent flags
static volatile sig_atomic_t prompt = false; // Whether the user should be prompted

// Child flags
static volatile sig_atomic_t hold = false; // Whether the process should hold
static volatile sig_atomic_t resume = false; // Whether the process should resume
static volatile sig_atomic_t terminate = false; // Whether the process should terminate

void parent_sigint_handler(int signo) {
    // write(1, "PARENT OF ALL RECEIVED SIGINT\n", strlen("PARENT OF ALL RECEIVED SIGINT\n"));
    prompt = true;
    // write(1, "PARENT OF ALL PROCESSED SIGINT\n", strlen("PARENT OF ALL PROCESSED SIGINT\n"));
}

void child_sigint_handler(int signo) {
    // write(1, "CHILD RECEIVED SIGINT\n", strlen("CHILD RECEIVED SIGINT\n"));
    hold = true;
    // write(1, "CHILD PROCESSED SIGINT\n", strlen("CHILD PROCESSED SIGINT\n"));
}

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
    killpg(0, SIGCONT);
    killpg(0, SIGTERM);
    leave(1);
}

void terminateProgramChild(void) {
    leave(1);
}

void continueProgramParent(void) {
    //COMBACK: LOG SIGCONT SENT TO PROCESS GROUP
    char info[2048] = {};
    snprintf(info, sizeof(info), "%s : %d", strsignal(SIGCONT), getpgrp());
    logEvent(getpid(), SIGNAL_SENT, info);
    killpg(0, SIGCONT);
}

bool checkParentAction(const char *path) {
    if (!prompt) return false;
    logEvent(getpid(), SIGNAL_RECV, strsignal(SIGINT));
    // for (int i = 0; i < number_of_children; ++i) while (waitpid(0, NULL, WUNTRACED) >= 0);
    fprintf(stderr, "%d ; %s ; %d ; %d\n", getpid(), path, number_of_files, number_of_modified_files);
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

void childHoldAction(const char *path) {
    logEvent(getpid(), SIGNAL_RECV, strsignal(SIGINT));
    fprintf(stderr, "%d ; %s ; %d ; %d\n", getpid(), path, number_of_files, number_of_modified_files);
    char info[2048] = {};
    snprintf(info, sizeof(info), "%s : %d", strsignal(SIGSTOP), getpid());
    logEvent(getpid(), SIGNAL_SENT, info);
    logEvent(getpid(), SIGNAL_RECV, strsignal(SIGSTOP));
    // for (int i = 0; i < number_of_children; ++i) while (waitpid(0, NULL, WUNTRACED) >= 0);
    // puts("RAISING SIGSTOP");
    raise(SIGSTOP);
    hold = false;
}

void childResumeAction(void) {
    logEvent(getpid(), SIGNAL_RECV, strsignal(SIGCONT));
    resume = false;
}

void childTerminateAction(void) {
    logEvent(getpid(), SIGNAL_RECV, strsignal(SIGTERM));
    terminate = false;
}

bool checkChildAction(const char *path) {
    if (hold) childHoldAction(path);
    if (resume) childResumeAction();
    if (terminate) {
        childTerminateAction();
        return true;
    }
    return false;
}

bool checkTerminateSignal(const char *path) {
    return isParentProcess() ? checkParentAction(path) : checkChildAction(path);
}

void terminateProgram(void) {
    isParentProcess() ? terminateProgramParent() : terminateProgramChild();
}