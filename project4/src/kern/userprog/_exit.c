#include <types.h>
#include <kern/errno.h>
#include <kern/unistd.h>
#include <thread.h>
#include <curthread.h>
#include <lib.h>

void sys__exit(int exitcode) {

	// Call thread.c's exit
	// We modified thread_exit to handle aditional cleanup.
	curthread->exitstatus = exitcode;
	thread_exit(exitcode);
}