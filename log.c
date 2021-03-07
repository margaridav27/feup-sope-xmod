//control with things relationated to log file
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

static int logFileFD;

bool check_log_filename(){
    char* logFile;

    if((logFile = getenv("LOG_FILENAME")) != NULL){
        logFileFD = open(logFile,O_WRONLY | O_CREAT | O_TRUNC);
        write(logFileFD,logFile,strlen(logFile));
        return true;
    }

    fprintf(stderr, "WARNING variavel LOG_FILENAME not defined.\n");
    return false;
}