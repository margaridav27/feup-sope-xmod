#ifndef PROJECT_INCLUDE_SIGNALS_H_
#define PROJECT_INCLUDE_SIGNALS_H_

#include <stdbool.h>

void parent_sigint_handler(int signo);

void child_sigint_handler(int signo);

void child_sigcont_handler(int signo);

void child_sigterm_handler(int signo);

int setUpSignals(const char *p);

void terminateProgramParent(void);

void continueProgramParent(void);

bool checkParentAction(void);

void childHoldAction(const char *path);

void childResumeAction(void);

void childTerminateAction(void);

bool checkChildAction(void);

bool checkTerminateSignal(void);

void terminateProgram(void);

void log_current_status(const char *path, int number_of_files, int number_of_modified_files);

#endif // PROJECT_INCLUDE_SIGNALS_H_
