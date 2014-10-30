#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#include <types.h>
#include <machine/trapframe.h>

/*
 * Prototypes for IN-KERNEL entry points for system call implementations.
 */

int sys_reboot(int code);

// Processes
int sys_getpid(pid_t *retval);
int sys_execv(const char *path, const char ** argv);
pid_t sys_fork(struct trapframe *tf);
int sys_waitpid(pid_t pid, userptr_t returncode, int flags, pid_t *retval);
void sys__exit(int exitcode);

// File Management
int sys_dup2(int fidold, int fidnew, int* retval);
int sys_lseek(int fid, off_t offset, int whence, int* retval);
int sys_open(const char* path, int oflag, size_t* retval);
int sys_read(int fid, void* buffer, size_t nbytes, int* retval);
int sys_write(int fid, const void* buffer, size_t nbytes, int* retval);
int sys_close(int fid, int* retval);


#endif /* _SYSCALL_H_ */
