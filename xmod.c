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

//file2 & file1 to test -R flag
//define environment variavel "export LOG_FILENAME="./trace.txt""

int main(int argc, char const *argv[])
{
    bool logFile = check_log_filename();
    if(logFile)
        printf("Ola");

    return 0;
}
