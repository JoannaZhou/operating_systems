/*
 * Sample/test code for running a user program.  You can use this for
 * reference when implementing the execv() system call. Remember though
 * that execv() needs to do more than this function does.
 */

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

/*
 * Load program "progname" and start running it in usermode.
 * Does not return except on error.
 *
 * Calls vfs_open on progname and thus may destroy it.
 */
int
runprogram(char *progname, char** arguments, int argument_count)
{
	struct vnode *v;
	vaddr_t entrypoint, stackptr;
	int result;
	
	kprintf("the program being run is %s\n", progname);

	/* Open the file. */
	result = vfs_open(progname, O_RDONLY, &v);
	if (result) {
		return result;
	}

	/* We should be a new thread. */
	assert(curthread->t_vmspace == NULL);

	/* Create a new address space. */
	curthread->t_vmspace = as_create();
	if (curthread->t_vmspace==NULL) {
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

	int i;
	int offset = 0;
	char* string_ptr[argument_count+1];

	string_ptr[argument_count] = NULL; 

	/* Iterate through each argument and copy it from kernel to user address */
	for(i = 0; i < argument_count; i++)
	{	
		//kprintf("length is %d\n", strlen(arguments[i]) +1);
		offset = offset + strlen(arguments[i]) + 1; //increment stack offset by string length
		//kprintf("offset is %d\n", offset);
		string_ptr[i] = stackptr - offset; //finding out address of string pointer on the stack
		//kprintf("string pointer is %d\n",string_ptr[i]);
		copyout(arguments[i], (userptr_t) string_ptr[i], strlen(arguments[i]) + 1);//copy string arguments from kernel to user address
	}
	
	int pointers_space = (argument_count + 1) * sizeof(char*); //calculate how much space is needed for all the string pointers
	//kprintf("pointer space is:%d\n", pointer_space);
	offset = offset + pointers_space; // increase offset to point to the string pointer
	//kprintf("offset is %d\n", offset);
	//kprintf("stack pointer is:%d\n", stackptr);
	int padding = (stackptr - offset) % 4; // calculate padding amount to make sure everything is aligned
	//kprintf("padding is:%d\n", padding);
	//kprintf("offset + padding is:%d\n", offset+padding);
	int total_space_needed = offset + padding;
	int address= stackptr - total_space_needed; // determine user level address
	copyout(string_ptr, (userptr_t) address, pointers_space); //copy string pointers from kernel to user space
	stackptr = address; //move down the stack

	/* Warp to user mode. */
	md_usermode(argument_count, (userptr_t)stackptr,
		    stackptr, entrypoint);
	
	/* md_usermode does not return */
	panic("md_usermode returned\n");
	return EINVAL;
}

