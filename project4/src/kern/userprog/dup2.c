#include <types.h>
#include <syscall.h>
#include <thread.h>
#include <curthread.h>
#include <filetable.h>
#include <../include/kern/errno.h>

int sys_dup2(int fidold, int fidnew, int* retval) {

	struct openfile* duplicatefile = (struct openfile*)ftGet(curthread->ft,fidnew);
	if (duplicatefile == NULL) {
		return EBADF;
	}

	if (ftArraySize(curthread->ft) == MAXFILETABLESIZE) {
		return EMFILE;
	}

	if (duplicatefile) {
		sys_close(fidnew,retval);
	}

	duplicatefile = (struct openfile*) ftGet(curthread->ft,fidold);
	
	if (duplicatefile == NULL) {
		return EBADF;
	}

	ftSet(curthread->ft,fidnew,(void*)duplicatefile);

	*retval = fidold;

	return 0;
}