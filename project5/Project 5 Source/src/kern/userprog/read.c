#include <types.h>
#include <syscall.h>
#include <thread.h>
#include <curthread.h>
#include <../include/kern/errno.h>
#include <uio.h>
#include <filetable.h>

int sys_read(int fid, void* buffer, size_t nbytes, int* retval) {
	if(!buffer || buffer == NULL) {
		return EFAULT;
	}

	struct openfile* readfile = (struct openfile*)ftGet(curthread->ft,fid);
	
	if (readfile == NULL) {
		return EBADF;
	}

	struct uio userio;
	mk_kuio(&userio, buffer,nbytes,readfile->offset,UIO_READ);

	int error = VOP_READ(readfile->data,&userio);

	if (error) {
		return error;
	}

	readfile->offset = userio.uio_offset;

	*retval = userio.uio_resid;

	return 0;
}