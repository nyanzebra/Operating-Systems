#include <types.h>
#include <syscall.h>
#include <thread.h>
#include <curthread.h>
#include <../include/kern/errno.h>
#include <uio.h>
#include <filetable.h>
#include <addrspace.h>

int sys_write(int fid, const void* buffer, size_t nbytes, int* retval) {
	if (!buffer || buffer == NULL) {
		return EFAULT;
	}

	struct openfile* writefile = (struct openfile*)ftGet(curthread->ft,fid);
	
	if (writefile == NULL) {
		return EBADF;
	}

	struct uio userio;

	mk_kuio(&userio,buffer,nbytes,writefile->offset, UIO_WRITE);

	int error = VOP_WRITE(writefile->data,&userio);

	if (error) {
		return error;
	}
	
	writefile->offset = userio.uio_offset;

	*retval = userio.uio_resid;

	return 0;
}