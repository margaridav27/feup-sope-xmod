#include <signal.h> // signal(), raise(), SIG_DFL, killpg()
#include <unistd.h> // usleep(), read(), write(), fsync(),
#include <string.h> // strlen()
#include <sys/wait.h>
#include <stdio.h>

#include "../include/signals.h"
#include "../include/utils.h" // isParentProcess(), leave()
#include "../include/log.h" // logSignalReceived(), logCurrentStatus(), logSignalSent()

const char *path;
extern int numberOfFilesFound, numberOfModifiedFiles, numberOfChildren;
const int *const noFiles = &numberOfFilesFound;
const int *const noModifiedFiles = &numberOfModifiedFiles;
const int *const noChildren = &numberOfChildren;

sig_atomic_t prompt = false;

void generic_signal_handler(int sig_no) {
    logSignalReceived(sig_no);

    struct sigaction newAction;

    newAction.sa_handler = SIG_DFL; // select default handler for signal
    sigemptyset(&newAction.sa_mask);
    newAction.sa_flags = 0;

    // Our own actions
    if (sig_no == SIGINT) {
        isParentProcess() ? parentSigintHandler() : childSigintHandler();
    } else if (sig_no == SIGUSR1 && !isParentProcess()) {
        leave(1);
    } else {
        // Repeat the requested actions
        sigaction(sig_no, &newAction, NULL);
        raise(sig_no);
    }
}

void parentSigintHandler(void) {
    if (prompt) return;
    prompt = true;
    // Wait until all children have paused
    int noPaused = 0;
    while (noPaused < *noChildren && waitpid(0, NULL, WUNTRACED) >= 0) ++noPaused;
    logCurrentStatus(path, *noFiles, *noModifiedFiles);
    char c = 'Y';
    do {
        const char *prompt = "Are you sure that you want to terminate? (Y/N) ";
        write(STDOUT_FILENO, prompt, strlen(prompt));
        fsync(STDOUT_FILENO);
        int n = read(STDIN_FILENO, &c, 1);
        if (n == -1) break;
    } while (c != 'Y' && c != 'y' && c != 'N' && c != 'n');
    prompt = false;
    if (c == 'Y' || c == 'y') terminateProgramParent();
    else if (c == 'N' || c == 'n') continueProgramParent();
}

void childSigintHandler(void) {
    logCurrentStatus(path, *noFiles, *noModifiedFiles);
    int noPaused = 0;
    while (noPaused < *noChildren && waitpid(0, NULL, WUNTRACED) >= 0) ++noPaused;
    logSignalSent(SIGSTOP, getpid());
    logSignalReceived(SIGSTOP);
    raise(SIGSTOP);
}

int setUpSignals(const char *_path) {
    struct sigaction newActionTerm, newActionAll;

    newActionTerm.sa_handler = SIG_IGN; // no handler specified
    sigemptyset(&newActionTerm.sa_mask);
    newActionTerm.sa_flags = 0;

    newActionAll.sa_handler = generic_signal_handler; // handler to print signal received message
    sigemptyset(&newActionAll.sa_mask);
    newActionAll.sa_flags = SA_RESTART; // Useful in case we interrupt a system call

    path = _path;
    int r = 0;
    for (int sig_no = 1; sig_no < SIGRTMIN; ++sig_no) {
        if (sig_no == SIGKILL || sig_no == SIGSTOP || sig_no == SIGCHLD) continue;
        r = sigaction(sig_no, &newActionAll, NULL);// r set to -1 in case sigaction return error
    }
    if (isParentProcess() && sigaction(SIGUSR1, &newActionTerm, NULL)) r = -1;
    return r;
}

void terminateProgramParent(void) {
    killpg(0, SIGCONT); // Wake up children
    logSignalSent(SIGCONT, getpgrp());
    killpg(0, SIGUSR1); // Ask children to terminate
    logSignalSent(SIGUSR1, getpgrp());
    leave(1); // Abnormal termination: exit code 1
}

int continueProgramParent(void) {
    if (killpg(0, SIGCONT) == -1) return -1; // Wake up children
    if (logSignalSent(SIGCONT, getpgrp())) return -1;
    return 0;
}
