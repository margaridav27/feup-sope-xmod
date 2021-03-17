//COMBACK: Explain header usages
#ifndef PROJECT_INCLUDE_SIGNALS_H_
#define PROJECT_INCLUDE_SIGNALS_H_

#include <stdbool.h>

void parent_sigint_handler(int signo);

void child_sigint_handler(int signo);

int setUpSignals(const char *p);

void terminateProgramParent(void);

void continueProgramParent(void);

#endif // PROJECT_INCLUDE_SIGNALS_H_
