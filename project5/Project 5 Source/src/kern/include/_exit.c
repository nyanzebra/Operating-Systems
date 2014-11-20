#include <types.h>
#include <kern/errno.h>
#include <kern/unistd.h>
#include <thread.h>
#include <curthread.h>
#include <lib.h>

void sys__exit(int exitcode) {
    /*
    The code for freeing resources used by a process are found in thread.c
    since we need to be able to free these resources on processes that exit
    abnormally (crash without calling _exit())
    */
	curthread->exit_status = exitcode;
	thread_exit();
}