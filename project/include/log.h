#ifndef PROJECT_INCLUDE_LOG_H_
#define PROJECT_INCLUDE_LOG_H_

#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>

typedef enum {
    PROC_CREAT,
    PROC_EXIT,
    SIGNAL_RECV,
    SIGNAL_SENT,
    FILE_MODF
} event_t;

/**
 * @brief
 *
 * @param flag
 * @return int
 */
int openLogFile(char *flag);

/**
 * @brief
 * 
 * @param pid
 * @param event
 * @param info
 * @return int
 */
int logEvent(pid_t pid, event_t event, char *info);

/**
 * @brief Closes logfile
 * 
 * @return 0 if it succeeds, 1 otherwise
 */
int closeLogFile();

#endif // PROJECT_INCLUDE_LOG_H_
