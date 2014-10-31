#ifndef _FILETABLE_H_
#define _FILETABLE_H_

#include <array.h>

#define MAXFILETABLESIZE 50

struct openfile;
struct filetable;

struct openfile {
	struct vnode* data;
	off_t offset;
	int fid;
	int mode;
	int numOwners;
};

struct filetable {
	struct array* files;
	int size;
};

struct filetable* ftCreate();
int ftAttachstds(struct filetable* ft);
int ftArraySize(struct filetable *ft);
int ftSize(struct filetable * ft);
struct openfile* ftGet(struct filetable* ft, int fti);
int ftSet(struct filetable* ft, struct openfile* fdn, int fti);
int ftAdd(struct filetable* ft, struct openfile* fdn);
int ftRemove(struct filetable* ft, int fti);
int ftDestroy(struct filetable* ft);

#endif
