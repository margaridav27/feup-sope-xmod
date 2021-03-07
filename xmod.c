#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include "log.h"

static bool logFileAvailable;

int main(int argc, char const *argv[])
{
    logFileAvailable = check_log_filename();

    return 0;
}
