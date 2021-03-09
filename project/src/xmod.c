#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "log.h"
#include "parse.h"
#include "time_ctrl.h"

static bool logFileAvailable;

int main(int argc, char *argv[]) {
    setBegin();

    command_t result;
    if (parseCommand(argc, argv, &result)) return 1;

    logFileAvailable = checkLogFilename();
    if (logFileAvailable) {
        registerEvent(getpid(), FILE_MODF, "some additional info");
    } else {
        printf("File not available. Could not register event.\n");
    }

    return 0;
}
