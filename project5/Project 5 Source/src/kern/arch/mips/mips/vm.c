// vm.c
// Created by: Jonathan Hart (jch0063)
// For: COMP 3500 Project 5
// Related files: vm.h, addrspace.h, coremap.h

#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <thread.h>
#include <curthread.h>
#include <addrspace.h>
#include <vm.h>
#include <machine/spl.h>
#include <machine/tlb.h>

size_t
vm_bootstrap(void)
{
	/* Do nothing. */

	// Initialize coremap
	coremap_bootstrap();

	// Create global page lock
	global_paging_lock = lock_create("global_paging_lock");

	// Return size of memory
	return mips_ramsize();
}

int
vm_fault(int faulttype, vaddr_t faultaddress) {
	struct addrspace *as;

	faultaddress &= PAGE_FRAME;
	assert(faultaddress < MIPS_KSEG0);

	as = curthread->t_vmspace;

	if (as == NULL) {
		return EFAULT;
	}

	return as_fault(as, faulttype, faultaddress);
}
