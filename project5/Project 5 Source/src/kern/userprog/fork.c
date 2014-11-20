#include <kern/unistd.h>
#include <types.h>
#include <thread.h>
#include <curthread.h>
#include <lib.h>
#include <kern/errno.h>
#include <machine/pcb.h>
#include <machine/spl.h>
#include <machine/trapframe.h>
#include <array.h>

static
void
child_thread(void *vtf, unsigned long junk)
{
    struct trapframe mytf;
    struct trapframe *ntf = vtf;

    (void)junk;

    /*
     * Now copy the trapframe to our stack, so we can free the one
     * that was malloced and use the one on our stack for going to
     * userspace.
     */

    mytf = *ntf;
    kfree(ntf);

    md_forkentry(&mytf);
}

pid_t sys_fork(struct trapframe *tf, pid_t *retval) {

    // Create a new trap frame
    struct trapframe *new_tf;

    int returnValue;

    // Copy the trapframe
    // Allocate memory for the new trapframe
    new_tf = kmalloc(sizeof(struct trapframe));

    // Error handling: if kmalloc does not succeed, return ENOMEM
    if (new_tf == NULL) {
        return ENOMEM;
    }

    // Assign new trap frame the old trap frame
    *new_tf = *tf;

    // Call thread_fork to handle creation of new thread
    returnValue = thread_fork(curthread->t_name, child_thread, new_tf, 0, retval);

    // Check if thread_fork was successful
    if (returnValue) {
        kfree(new_tf);
        return returnValue;
    } else {
        return 0;
    }

}

