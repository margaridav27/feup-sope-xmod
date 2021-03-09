#include <time.h>

#include "../include/time_ctrl.h"

static clock_t begin;

void setBegin() {
    begin = clock();
}

int getElapsed() {
    clock_t now = clock();
    return (int) ((now - begin) / (CLOCKS_PER_SEC / 1000));
}
