#ifndef PID_H
#define PID_H

// Define the minimum PID > # of error codes
#define MIN_PROCESSES 100
#define MAX_PROCESSES 5000
#define MIN_PID 2
#define MAX_PID 5000
#define INVALID_PID	0	/* nothing has this pid */
#define BOOTUP_PID	1	/* first thread has this pid */

#include <types.h>
#include <synch.h>
#include <curthread.h>

struct pidinfo {
	// Bool for if thread has exited
    int isExited;
    // Exit status if thread has exited
    int exitStatus;
    // ID of thread
    pid_t t_pid;
    // ID of parent thread
    pid_t t_parent_pid;
    // CV to wait for the exit of thread
    struct cv *process_condition;
};

void initializePID(void);

int allocatePID(pid_t *retval);
void unallocatePID(pid_t theirpid);
void setPIDexitStatus(int status);
int PIDwait(pid_t targetpid, int *status, int flags, pid_t *retpid);

#endif
