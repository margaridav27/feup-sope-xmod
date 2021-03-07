#ifndef TIME_CTRL
#define TIME_CTRL

/**
 * @brief Marks the beginning of the program execution
 * 
 */
void setBegin();

/**
 * @brief Calculates the seconds elapsed since the beginning of the program execution
 * 
 * @return seconds elapsed 
 */
int getElapsed();

#endif