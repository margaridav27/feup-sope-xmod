#ifndef PROJECT_INCLUDE_SIGNALS_H_
#define PROJECT_INCLUDE_SIGNALS_H_

void parentSigintHandler(void);

void childSigintHandler(void);

int setUpSignals(const char *_path);

void terminateProgramParent(void);

int continueProgramParent(void);

#endif // PROJECT_INCLUDE_SIGNALS_H_
