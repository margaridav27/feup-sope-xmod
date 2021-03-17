#ifndef PROJECT_INCLUDE_SIGNALS_H_
#define PROJECT_INCLUDE_SIGNALS_H_

void parent_sigint_handler(void);

void child_sigint_handler(void);

int setUpSignals(const char *p);

void terminateProgramParent(void);

int continueProgramParent(void);

#endif // PROJECT_INCLUDE_SIGNALS_H_
