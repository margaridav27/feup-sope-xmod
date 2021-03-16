#include "../include/signals.h"
#include <signal.h>
#include <stdbool.h>

// Parent flags
static volatile sig_atomic_t prompt = false; // Whether the user should be prompted

// Child flags
static volatile sig_atomic_t hold = false; // Whether the process should hold
static volatile sig_atomic_t resume = false; // Whether the process should resume
static volatile sig_atomic_t terminate = false; // Whether the process should terminate