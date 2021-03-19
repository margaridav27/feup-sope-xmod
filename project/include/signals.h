#ifndef PROJECT_INCLUDE_SIGNALS_H_
#define PROJECT_INCLUDE_SIGNALS_H_

int setUpSignals(const char *_path);

void genericSignalHandler(int sig_no);

void parentSigintHandler(void);

void childSigintHandler(void);

int terminateProgramParent(void);

int continueProgramParent(void);

#endif // PROJECT_INCLUDE_SIGNALS_H_