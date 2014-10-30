#include <machine/spl.h>
#include <kern/unistd.h>
#include <pid.h>
#include <thread.h>
#include <curthread.h>
#include <vm.h>
#include <kern/errno.h>
#include <lib.h>

#include <addrspace.h>

int sys_waitpid(pid_t pid, userptr_t retstatus, int flags, pid_t *retval)
{
    int status; 
    int result;

    result = PIDwait(pid, &status, flags, retval);
    if (result) {
        return result;
    }

    return copyout(&status, retstatus, sizeof(int));
}

