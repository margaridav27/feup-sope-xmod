#include "../include/signals.h"
#include "../include/utils.h"
#include <signal.h>
#include <stdbool.h>

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