#ifndef PID_H
#define PID_H

#define MIN_PID 100

#include <types.h>

pid_t newPid();
void pidProcessExit(pid_t pid);
void pidParentDone(pid_t pid);
void pidFree(pid_t pid);
int pidClaimed(pid_t pid);

#endif
