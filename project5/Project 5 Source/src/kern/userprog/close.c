#include <types.h>
#include <syscall.h>
#include <thread.h>
#include <curthread.h>
#include <filetable.h>
#include <kern/errno.h>

int sys_close(int fid, int* retval) {
	if (ftGet(curthread->ft,fid) == NULL) {
		return EBADF;
	}
	ftRemove(curthread->ft,fid);
	*retval = 0;
	return 0;
}
