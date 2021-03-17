#include "../include/signals.h"
#include <signal.h> // signal(), raise(), SIG_DFL, killpg()
#include <unistd.h> // usleep(), read(), write(), fsync(),
#include <string.h> // strlen()
#include "../include/utils.h" // is_parent_process(), leave()
#include "../include/log.h" // log_signal_received(), log_current_status(), log_signal_sent()

const char *path;
extern int number_of_files, number_of_modified_files;
const int *const no_files = &number_of_files;
const int *const no_modified_files = &number_of_modified_files;

void generic_signal_handler(int sig_no) {
    log_signal_received(sig_no);

    // Our own actions
    if (sig_no == SIGINT) {
        isParentProcess() ? parent_sigint_handler() : child_sigint_handler();
    } else if (sig_no == SIGTERM && !isParentProcess()) {
        leave(1);
    }
        // Repeat the requested actions
    else {
        signal(sig_no, SIG_DFL);
        raise(sig_no);
    }
}

void parent_sigint_handler(void) {
    // COMBACK: Find a better solution for syncronizing this
    usleep(5000); // Wait for other prompts
    //COMBACK: Check signal safety
    log_current_status(path, *no_files, *no_modified_files);
    char c = 'Y';
    do {
        const char *prompt = "Are you sure that you want to terminate? (Y/N) ";
        write(STDOUT_FILENO, prompt, strlen(prompt));
        fsync(STDOUT_FILENO);
        int n = read(STDIN_FILENO, &c, 1);
        if (n == -1) break;
    } while (c != 'Y' && c != 'y' && c != 'N' && c != 'n');
    if (c == 'Y' || c == 'y') terminateProgramParent();
    else if (c == 'N' || c == 'n') continueProgramParent();
}

void child_sigint_handler(void) {
    log_current_status(path, *no_files, *no_modified_files);
    log_signal_sent(SIGSTOP, getpid());
    log_signal_received(SIGSTOP);
    //COMBACK: Try to find a way to use this to alert the parent that we are done printing. If it is not possible, 
    // pause the thread instead
    raise(SIGSTOP);
}

int setUpSignals(const char *p) {
    // COMBACK: Change signal setup to use sigaction
    // COMBACK: Find a better way to forward this argument
    path = p;
    int r = 0;
    for (int sig_no = 1; sig_no < SIGRTMIN; ++sig_no) {
        if (sig_no == SIGKILL || sig_no == SIGSTOP || sig_no == SIGCHLD) continue;
        if (signal(sig_no, generic_signal_handler) == SIG_ERR) r = -1;
    }
    // COMBACK: Stop using SIGTERM as the termination signal, convert to SIGUSR
    if (isParentProcess() && signal(SIGTERM, SIG_IGN)) r = -1;
    return r;
}

void terminateProgramParent(void) {
    killpg(0, SIGCONT); // Wake up children
    log_signal_sent(SIGCONT, getpgrp());
    killpg(0, SIGTERM); // Ask children to terminate
    log_signal_sent(SIGTERM, getpgrp());
    leave(1); // Abnormal termination: exit code 1
}

int continueProgramParent(void) {
    if (killpg(0, SIGCONT) == -1) return -1; // Wake up children
    if (log_signal_sent(SIGCONT, getpgrp())) return -1;
    return 0;
}
