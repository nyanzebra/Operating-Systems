#include <types.h>
#include <syscall.h>
#include <thread.h>
#include <curthread.h>
#include <../include/kern/errno.h>
#include <../include/kern/unistd.h>

int sys_lseek(int fid, off_t offset, int whence, int* retval) {

	struct openfile* seekfile = (struct openfile*)array_getguy(curthread->t_openfiletable,fid);
	if (seekfile == NULL) {
		return EBADF;
	}
	
	if (whence == SEEK_SET) {
		offset = offset;
	} else if(whence == SEEK_CUR) {
		offset = seekfile->offset + offset;
	} else if (whence == SEEK_END) {
		//end of file plus offset
	} else {
		return EINVAL;
	}

	*retval = VOP_TRYSEEK(seekfile->data,offset);
	if (*retval < 0) {
		return EINVAL;
	}

	return 0;
}