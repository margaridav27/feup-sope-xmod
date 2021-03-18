#include "../include/signals.h"

#include <signal.h> // signal(), raise(), SIG_DFL, killpg()
#include <string.h> // strlen()
#include <sys/wait.h> // waitpid, WUNTRACED
#include <unistd.h> // usleep(), read(), write(), fsync(),

#include "../include/log.h" // logSignalReceived(), logCurrentStatus(), logSignalSent()
#include "../include/utils.h" // isParentProcess(), leave()

const char *path;
extern int number_of_files_found, number_of_modified_files, number_of_children;
const int *const _number_of_files_found = &number_of_files_found;
const int *const _number_of_modified_files = &number_of_modified_files;
const int *const _number_of_children = &number_of_children;

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
    int number_of_paused = 0;
    while (number_of_paused < *_number_of_children && waitpid(0, NULL, WUNTRACED) >= 0) ++number_of_paused;
    logCurrentStatus(path, *_number_of_files_found, *_number_of_modified_files);
    char c = 'Y';
    do {
        const char *s = "Are you sure that you want to terminate? (Y/N) ";
        write(STDOUT_FILENO, s, strlen(s));
        fsync(STDOUT_FILENO);
        int n = read(STDIN_FILENO, &c, 1);
        if (n == -1) break;
    } while (c != 'Y' && c != 'y' && c != 'N' && c != 'n');
    prompt = false;
    if (c == 'Y' || c == 'y') terminateProgramParent();
    else if (c == 'N' || c == 'n') continueProgramParent();
}

void childSigintHandler(void) {
    logCurrentStatus(path, *_number_of_files_found, *_number_of_modified_files);
    int number_of_paused = 0;
    while (number_of_paused < *_number_of_children && waitpid(0, NULL, WUNTRACED) >= 0) ++number_of_paused;
    logSignalSent(SIGSTOP, getpid());
    logSignalReceived(SIGSTOP);
    raise(SIGSTOP);
}

int setUpSignals(const char *_path) {
    struct sigaction terminate_action, generic_action;

    terminate_action.sa_handler = SIG_IGN; // no handler specified
    sigemptyset(&terminate_action.sa_mask);
    terminate_action.sa_flags = 0;

    generic_action.sa_handler = generic_signal_handler; // handler to print signal received message
    sigemptyset(&generic_action.sa_mask);
    generic_action.sa_flags = SA_RESTART; // Useful in case we interrupt a system call

    path = _path;
    int r = 0;
    for (int sig_no = 1; sig_no < SIGRTMIN; ++sig_no) {
        if (sig_no == SIGKILL || sig_no == SIGSTOP || sig_no == SIGCHLD) continue;
        r = sigaction(sig_no, &generic_action, NULL);// r set to -1 in case sigaction return error
    }
    if (isParentProcess() && sigaction(SIGUSR1, &terminate_action, NULL)) r = -1;
    return r;
}

int terminateProgramParent(void) {
    pid_t pgrp = getpgrp();
    if (killpg(0, SIGCONT)) return -1; // Wake up children
    if (logSignalSent(SIGCONT, pgrp)) return -1;
    if (killpg(0, SIGUSR1)) return -1; // Ask children to terminate
    if (logSignalSent(SIGUSR1, pgrp)) return -1;
    leave(1); // Abnormal termination: exit code 1
    return 0;
}

int continueProgramParent(void) {
    if (killpg(0, SIGCONT) == -1) return -1; // Wake up children
    if (logSignalSent(SIGCONT, getpgrp())) return -1;
    return 0;
}
