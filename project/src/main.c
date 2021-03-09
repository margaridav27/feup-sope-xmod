#include <stdbool.h>
#include <stdio.h>

#include "log.h"
#include "parse.h"
#include "time_ctrl.h"
#include "xmod.h"

static bool logFileAvailable;

int main(int argc, char *argv[]) {
    setBegin();

    logFileAvailable = checkLogFilename();
    if (logFileAvailable) registerEvent(getpid(), FILE_MODF, "some additional info");
    else
        printf("File not available. Could not register event.\n");

    command_t result;
    if (parseCommand(argc, argv, &result)) return 1;
    printRetainMessage(&result);
    printChangeMessage(455, &result); //455 = 0707
    //changeMode(&result);
    return 0;
}
