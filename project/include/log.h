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
 * @brief Checks LOG_FILENAME variable content  
 * @brief Opens the correspondent file if its path is defined

 * @return 0 if the variable is defined and the file is open successfully, 1 otherwise
 */
int initLog(char *flag);

/**
 * @brief Registers event at the file defined by LOG_FILENAME
 * 
 * @param pid process ID of the process that makes the register
 * @param event type of event to be registered
 * @param info additional info associated with the event
 */
int registerEvent(int pid, event_t event, char *info);

#endif  // PROJECT_INCLUDE_LOG_H_
