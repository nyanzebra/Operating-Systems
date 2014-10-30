#include <types.h>
#include <kern/unistd.h>
#include <kern/errno.h>
#include <lib.h>
#include <addrspace.h>
#include <thread.h>
#include <curthread.h>
#include <vm.h>
#include <vfs.h>
#include <test.h>

int
sys_execv(const char *path, const char ** argv)
{
	// Originally based on runprogram.c
	struct vnode *v;
	vaddr_t entrypoint, stackptr;
	int result;

	if(path == NULL)
	{
		return EFAULT;
	}
	
	char *fname = (char *)kmalloc(1024);
	size_t size;

	copyinstr( (userptr_t)path, fname, 1024, &size);	

	// Determine number of arguments
	int i = 0;
	while( argv[i] != NULL ) {
  		i++;
	}

  	int argc = i;
	
	// Create new arguments array
	char **args;
	argv = (char **)kmalloc(sizeof(char*));

	// Copy in all the arguments from passed in args
	size_t argsLength;

	for(i = 0; i < argc; i++) {
		int len = strlen(argv[i]);
		len++;
		args[i] = (char*)kmalloc(len);
		copyinstr( (userptr_t)argv[i], args[i], len, &argsLength );
  	}

  	// Set the end of the new arguments
  	args[argc] = NULL;
	
	/* Open the file. */
	result = vfs_open(path, O_RDONLY, &v);
	if (result) {
		return result;
	}

	// Destroy the current address space
	if(curthread->t_vmspace != NULL)
	{
		as_destroy(curthread->t_vmspace);
		curthread->t_vmspace = NULL;
	}

	/* We should be a new thread. */
	assert(curthread->t_vmspace == NULL);

	/* Create a new address space. */
	curthread->t_vmspace = as_create();
	if (curthread->t_vmspace == NULL) {
		vfs_close(v);
		return ENOMEM;
	}

	/* Activate it. */
	as_activate(curthread->t_vmspace);

	/* Load the executable. */
	result = load_elf(v, &entrypoint);
	if (result) {
		/* thread_exit destroys curthread->t_vmspace */
		vfs_close(v);
		return result;
	}

	/* Done with the file now. */
	vfs_close(v);

	/* Define the user stack in the address space */
	result = as_define_stack(curthread->t_vmspace, &stackptr);
	if (result) {
		/* thread_exit destroys curthread->t_vmspace */
		return result;
	}

	//set up the arguments in the user stack
	  	
	//copy the contents to stack
	unsigned int programStack[argc];

	for(i = argc-1; i >= 0; i--) {
		int argLen = strlen(args[i]);
		int shift = (argLen % 4);
		if(shift == 0)
			shift = 4;
		stackptr = stackptr - (argLen + shift);
		copyoutstr( args[i], (userptr_t)stackptr, argLen, &argsLength );
		programStack[i] = stackptr;
	}
	
	programStack[argc] = (int)NULL;
	for(i = argc-1; i >= 0; i--)
	{
		stackptr = stackptr - 4;
		copyout(&programStack[i] ,(userptr_t)stackptr, sizeof(programStack[i]));
	}

	/* Warp to user mode. */
	md_usermode(0 /*argc*/, NULL /*userspace addr of argv*/,
		    stackptr, entrypoint);
	
	/* md_usermode does not return */
	panic("md_usermode returned\n");
	return EINVAL;
}
