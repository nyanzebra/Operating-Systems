// vm.h
// Created by: Jonathan Hart (jch0063)
// For: COMP 3500 Project 5
// Related files: vm.h, addrspace.h, coremap.h, lpage.c, swap.c

#ifndef _VM_H_
#define _VM_H_

#include <machine/vm.h>

/*
 * VM system-related definitions.
 *
 * You'll probably want to add stuff here.
 */


/* Fault-type arguments to vm_fault() */
#define VM_FAULT_READ        0    /* A read was attempted */
#define VM_FAULT_WRITE       1    /* A write was attempted */
#define VM_FAULT_READONLY    2    /* A write to a readonly page was attempted*/


/* Initialization function */
size_t vm_bootstrap(void);

/* Fault handling function called by trap code */
int vm_fault(int faulttype, vaddr_t faultaddress);

/* Allocate/free kernel heap pages (called by kmalloc/kfree) */
vaddr_t alloc_kpages(int npages);
void free_kpages(vaddr_t addr);


struct addrspace;

/////////////////////////////////////////////////////////////////
/// lpage - logical page
struct lpage {
	paddr_t lp_paddr;
	off_t lp_swapaddr;
};

// lpage flags
#define LPF_DIRTY	0x1
#define LPF_LOCKED	0x2
#define LPF_MASK	0x3

#define LP_ISDIRTY(lp)		((lp)->lp_paddr & LPF_DIRTY)
#define LP_ISLOCKED(lp)		((lp)->lp_paddr & LPF_LOCKED)

#define LP_SET(am, bit)		((lp)->lp_paddr |= (bit))
#define LP_CLEAR(am, bit)	((lp)->lp_paddr &= ~(paddr_t)(bit))

// lpage functions
struct lpage *lpage_create(void);
void lpage_destroy(struct lpage *lp);
void lpage_lock(struct lpage *lp);
void lpage_unlock(struct lpage *lp);

int lpage_copy(struct lpage *from, struct lpage **toret);
int lpage_zerofill(struct lpage **lpret);
int lpage_fault(struct lpage *lp, struct addrspace *, int faulttype, vaddr_t va);
void lpage_evict(struct lpage *victim);


/////////////////////////////////////////////////////////////////
/// vm_object - block of virtual memory
struct vm_object {
	struct array *vmo_lpages;
	vaddr_t vmo_base;
	size_t vmo_lower_redzone;
};

// vm_object functions
struct vm_object *vm_object_create(size_t npages);
int vm_object_copy(struct vm_object *vmo, struct addrspace *newas, struct vm_object **newvmo_ret);
int vm_object_setsize(struct addrspace *as, struct vm_object *vmo, int newnpages);
void vm_object_destroy(struct addrspace *as, struct vm_object *vmo);

/////////////////////////////////////////////////////////////////
/// swap - swapping functions

off_t swap_alloc(void);
void swap_free(off_t diskpage);

int swap_reserve(unsigned long npages);
void swap_unreserve(unsigned long npages);

void swap_pagein(paddr_t paddr, off_t swapaddr);
void swap_pageout(paddr_t paddr, off_t swapaddr);

// Define invalid swap address
#define INVALID_SWAPADDR	(0)

// Global paging lock
struct lock *global_paging_lock;

#endif /* _VM_H_ */
