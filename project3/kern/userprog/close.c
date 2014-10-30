#include <types.h>
#include <syscall.h>
#include <thread.h>
#include <curthread.h>
#include <../include/kern/errno.h>

int sys_close(int fid, int* retval) {
	if (array_getguy(curthread->t_openfiletable,fid) == NULL) {
		return EBADF;
	}
	array_remove(curthread->t_openfiletable,fid);
	*retval = 0;
	return 0;
}
