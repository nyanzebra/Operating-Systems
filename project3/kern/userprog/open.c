#include <types.h>
#include <syscall.h>
#include <thread.h>
#include <curthread.h>
#include <addrspace.h>
#include <array.h>

#include <../include/kern/errno.h>
#include <../include/kern/unistd.h>

int sys_open(const char* path, int oflag, fid_t* retval) {
	if(!curthread->t_openfiletable) { // if the map does not exist yet then create it
		curthread->t_openfiletable = array_create();
	}

	if (oflag > 2 || oflag < 0) {
		return EINVAL;
	}

	if(array_getnum(curthread->t_openfiletable) == MAXFILETABLESIZE) {
		return EMFILE;
	}

	if ((oflag & O_CREAT) && (oflag & O_EXCL)) {
		int i = 0;
		int count = array_getnum(curthread->t_openfiletable);
		for (i = 0; i < count; ++i) {
			if (array_getguy(curthread->t_openfiletable,i) != NULL) {
				return EEXIST;
			}
		}
	}

	struct openfile* newfile;

	int error = vfs_open(path,oflag,newfile->data); // set the data for the file
	if (error) { //this takes care of a lot of errors
		return error;
	}
	newfile->offset = 0; // initial offset is 0 for beginning of file
	newfile->data->vn_countlock = lock_create(path); // mutex is on the path

	*retval = array_add(curthread->t_openfiletable,(void *)newfile); // add to map

	return 0;
}
