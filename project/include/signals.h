void parent_sigint_handler(int signo);

void child_sigint_handler(int signo);

void child_sigcont_handler(int signo);

void child_sigterm_handler(int signo);

int setUpSignals(void);