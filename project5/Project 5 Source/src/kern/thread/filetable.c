#include <types.h>
#include <kern/errno.h>
#include <kern/unistd.h>
#include <kern/limits.h>
#include <machine/spl.h>
#include <lib.h>
#include <array.h>
#include <queue.h>
#include <vfs.h>
#include <vnode.h>
#include <filetable.h>

struct filetable *ftCreate() {
   struct filetable *ft = kmalloc(sizeof (struct filetable));
   if (ft == NULL) {
       return NULL;
   }
   ft->size = 0;
   ft->files = array_create();
   if (ft->files == NULL) {
       ftDestroy(ft);
       return NULL;
   }
   array_preallocate(ft->files, 20);
   array_add(ft->files, NULL);
   array_add(ft->files, NULL);
   array_add(ft->files, NULL);
   return ft;
}

int ftAttachstds(struct filetable *ft) {
   char *console = NULL;
   int mode;
   int result = 0;
   struct vnode *vn_stdin;
   mode = O_RDONLY;
   struct openfile *fd_stdin = NULL;
   fd_stdin = (struct openfile *) kmalloc(sizeof ( struct openfile));
   if (fd_stdin == NULL) {
       ftDestroy(ft);
       return 0;
   }
   console = kstrdup("con:");
   result = vfs_open(console, mode, &vn_stdin);
   if (result) {
       vfs_close(vn_stdin);
       ftDestroy(ft);
       return 0;
   }
   kfree(console);
   fd_stdin->mode = mode;
   fd_stdin->offset = 0;
   fd_stdin->data = vn_stdin;
   fd_stdin->numOwners = 1;
   ftSet(ft, fd_stdin, STDIN_FILENO);
   struct vnode *vn_stdout;
   mode = O_WRONLY;
   struct openfile *fd_stdout = NULL;
   fd_stdout = (struct openfile *) kmalloc(sizeof (struct openfile));
   if (fd_stdout == NULL) {
       ftDestroy(ft);
       return 0;
   }
   console = kstrdup("con:");
   result = vfs_open(console, mode, &vn_stdout);
   if (result) {
       vfs_close(vn_stdout);
       ftDestroy(ft);
       return 0;
   }
   kfree(console);
   fd_stdout->mode = mode;
   fd_stdout->offset = 0;
   fd_stdout->data = vn_stdout;
   fd_stdout->numOwners = 1;
   ftSet(ft, fd_stdout, STDOUT_FILENO);
   struct vnode *vn_stderr;
   mode = O_WRONLY;
   struct openfile *fd_stderr = NULL;
   fd_stderr = (struct openfile *) kmalloc(sizeof (struct openfile));
   if (fd_stderr == NULL) {
       ftDestroy(ft);
       return 0; 
   }
   console = kstrdup("con:");
   result = vfs_open(console, mode, &vn_stderr);
   if (result) {
       vfs_close(vn_stderr);
       ftDestroy(ft);
       return 0;
   }
   kfree(console);
   fd_stderr->mode = mode;
   fd_stderr->offset = 0;
   fd_stderr->data = vn_stderr;
   fd_stderr->numOwners = 1;
   ftSet(ft, fd_stderr, STDERR_FILENO);
   return 1;
}

int ftArraySize(struct filetable *ft) {
   assert(ft != NULL);
   return (array_getnum(ft->files));
}

int ftSize(struct filetable *ft) {
   assert(ft != NULL);
   int total = array_getnum(ft->files);
   int i = 0;
   for (i = 0; i < ftArraySize(ft); i++) {
       if (ftGet(ft, i) == NULL) {
           total--;
       }
   }
   return total;
}

struct openfile *ftGet(struct filetable *ft, int fti) {
   if (fti < 0) {
       return NULL;
   }
   if (fti < 3) {
       if (array_getguy(ft->files, fti) == NULL) {
           ftAttachstds(ft);
       }
   }
   //Doesn't exist.
   if (fti >= ftArraySize(ft)) { 
     return NULL;
   }
   struct openfile *ret = array_getguy(ft->files, fti);
   return ret;
}

int ftSet(struct filetable* ft, struct openfile* fd, int fti) {
   if (fti >= ftArraySize(ft)) {
       return 1;
   }
   array_setguy(ft->files, fti, fd);
   if (ftGet(ft, fti) == fd) {
       return 1;
   }
   return 0;
}

int ftAdd(struct filetable* ft, struct openfile* fd) {
   int fdn = 0;
   for (fdn = 0; fdn < ftArraySize(ft) && fdn < OPEN_MAX; fdn++) {
       if (ftGet(ft, fdn) == NULL) {
           array_setguy(ft->files, fdn, fd);
           return fdn;
       }
   }
   if (fdn == OPEN_MAX) {
       return -1;
   }
   if (array_add(ft->files, fd) != 0) { 
       return -1;
   }
   fd->numOwners++;
   assert(fdn != 0);
   return fdn;
}

int ftRemove(struct filetable* ft, int fti) {
   struct openfile * fd = ftGet(ft, fti);
   if (fd != NULL) {
       int spl = splhigh();
       fd->numOwners--;
       if (fd->numOwners == 0) {
           vfs_close(fd->data);
           kfree(fd);
       }
       splx(spl);
       array_setguy(ft->files, fti, NULL);
   }
   return 1;
}

int ftDestroy(struct filetable* ft) {
   int i;
   for (i = ftArraySize(ft) - 1; i >= 0; i--) {
       ftRemove(ft, i);
   }
   kfree(ft);
   return 1;
}
