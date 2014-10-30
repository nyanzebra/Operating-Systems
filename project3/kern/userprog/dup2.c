#include <types.h>
#include <syscall.h>
#include <thread.h>
#include <curthread.h>
#include <../include/kern/errno.h>

int sys_dup2(int fidold, int fidnew, int* retval) {

	struct openfile* duplicatefile = (struct openfile*)array_getguy(curthread->t_openfiletable,fidnew);
	if (duplicatefile == NULL) {
		return EBADF;
	}

	if (array_getnum(&curthread->t_openfiletable) == MAXFILETABLESIZE) {
		return EMFILE;
	}

	if (duplicatefile) {
		sys_close(fidnew,retval);
	}

	duplicatefile = (struct openfile*) array_getguy(curthread->t_openfiletable,fidold);
	
	if (duplicatefile == NULL) {
		return EBADF;
	}

	array_setguy(curthread->t_openfiletable,fidnew,(void*)duplicatefile);

	*retval = fidold;

	return 0;
}