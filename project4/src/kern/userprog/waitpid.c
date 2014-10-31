#include <machine/spl.h>
#include <kern/unistd.h>
#include <pid.h>
#include <thread.h>
#include <curthread.h>
#include <vm.h>
#include <kern/errno.h>
#include <lib.h>
#include <childtable.h>
#include <addrspace.h>

int sys_waitpid(pid_t pid, int* status, int flags, pid_t *retval)
{
	if(*status % 4 != 0 ) {
		return EFAULT;
	}

	int spl = splhigh();
	if (flags != 0) {
		splx(spl);
		return EINVAL;
	}

    struct childtable* child = NULL;
    struct childtable* p = curthread->children;
    while (p != NULL) {
    	if (p->pid == pid) {
    		child = p;
    		break;
    	}
    	p = p->next;
    }
    if (child == NULL) {
    	splx(spl);
    	if(pidClaimed(pid)) {
    		return ESRCH;
    	} else {
    		return EINVAL;
    	}
    }

    while (child->finished == 0){
    	thread_sleep((void *)pid);
    }

    *status = child->exit_code;

    if (curthread->children->pid == pid) {
    	struct childtable* temp = curthread->children;
    	curthread->children = curthread->children->next;
    	kfree(temp);
    } else {
    	p = curthread->children;
    	while (1) {
    		assert(p->next != NULL);
    		if (p->next->pid == pid) {
    			struct childtable *temp = p->next;
    			p->next = p->next->next;
    			kfree(temp);
    			break;
    		}
    		p = p->next;
    	}
    }

    pidParentDone(pid);
    splx(spl);
    *retval = pid;
    return pid;
}

