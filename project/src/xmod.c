#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "log.h"
#include "time_ctrl.h"

static bool logFileAvailable;

int main(int argc, char const *argv[])
{
    setBegin();;
    
    logFileAvailable = checkLogFilename();

    if (logFileAvailable) { 
        registerEvent(getpid(), FILE_MODF, "some additional info");
    }
    else { 
        printf("File not available. Could not register event.\n"); 
    } 
    
    return 0;
}
