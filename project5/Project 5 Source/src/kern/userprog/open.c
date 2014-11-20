#include <types.h>
#include <syscall.h>
#include <thread.h>
#include <curthread.h>
#include <addrspace.h>
#include <filetable.h>
#include <kern/stat.h>
#include <kern/errno.h>
#include <kern/unistd.h>

int sys_open(const char* path, int oflag, size_t* retval) {
	if (oflag >= 63 || strlen(path) < 1) {
		return EINVAL;
	}

	if(ftArraySize(curthread->ft) == MAXFILETABLESIZE) {
		return EMFILE;
	}

	if ((oflag & O_CREAT) && (oflag & O_EXCL)) {
		int i = 0;
		for (i = 0; i < MAXFILETABLESIZE; ++i) {
			if (ftGet(curthread->ft,i) != NULL) {
				return EEXIST;
			}
		}
	}

	struct openfile* newfile = kmalloc(sizeof(struct openfile));
	newfile->data = kmalloc(sizeof(struct vnode));
	char* kfilename = kstrdup(path);
	int copyflags = oflag;
	oflag = oflag&O_ACCMODE;
	int offset = 0;
	int error = vfs_open(path,oflag,newfile->data); // set the data for the file
	if (error) { //this takes care of a lot of errors
		return error;
	}

	if (copyflags&O_APPEND) {
		struct stat *statbuf = kmalloc(sizeof(struct stat));
		VOP_STAT(newfile->data,statbuf);
		offset= statbuf->st_size;
		kfree(statbuf);
	}

	if(copyflags&O_TRUNC){
		VOP_TRUNCATE(newfile->data,0);
	}
	kfree(kfilename);
	newfile->offset = offset; // initial offset is 0 for beginning of file
	error = ftAdd(curthread->ft,newfile);
	*retval = error; // add to map

	return 0;
}
