#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <machine/pcb.h>
#include <machine/spl.h>
#include <machine/trapframe.h>
#include <kern/callno.h>
#include <syscall.h>
#include <uio.h>
#include <kern/unistd.h>
#include <vnode.h>
#include <synch.h>
#include <thread.h>
#include <curthread.h>
#include <addrspace.h>
#include <vfs.h>



int numthreads;

/*
 * System call handler.
 *
 * A pointer to the trapframe created during exception entry (in
 * exception.S) is passed in.
 *	
 * The calling conventions for syscalls are as follows: Like ordinary
 * function calls, the first 4 32-bit arguments are passed in the 4
 * argument registers a0-a3. In addition, the system call number is
 * passed in the v0 register.
 *
 * On successful return, the return value is passed back in the v0
 * register, like an ordinary function call, and the a3 register is
 * also set to 0 to indicate success.
 *
 * On an error return, the error code is passed back in the v0
 * register, and the a3 register is set to 1 to indicate failure.
 * (Userlevel code takes care of storing the error code in errno and
 * returning the value -1 from the actual userlevel syscall function.
 * See src/lib/libc/syscalls.S and related files.)
 *
 * Upon syscall return the program counter stored in the trapframe
 * must be incremented by one instruction; otherwise the exception
 * return code will restart the "syscall" instruction and the system
 * call will repeat forever.
 *
 * Since none of the OS/161 system calls have more than 4 arguments,
 * there should be no need to fetch additional arguments from the
 * user-level stack.
 *
 * Watch out: if you make system calls that have 64-bit quantities as
 * arguments, they will get passed in pairs of registers, and not
 * necessarily in the way you expect. We recommend you don't do it.
 * (In fact, we recommend you don't use 64-bit quantities at all. See
 * arch/mips/include/types.h.)
 */

static struct lock *cons_lock=NULL;
static struct lock *fork_lock=NULL;

int sys_write(int fd, userptr_t buf, size_t size, int *retval){
	
	//kprintf("numthreads is %d\n", numthreads);
	if(fd!=STDOUT_FILENO){
		return EBADF;
	}
	
	//lock to write/read to/from console
	lock_acquire(cons_lock);
	
	//declare userspace
	struct uio user;		
	struct vnode *console_vnode;
	
	//set up vnode for console
	char *console = kstrdup("con:");
	int ret = vfs_open(console, O_WRONLY, &console_vnode);
	kfree(console);
	
	//set up userspace
	mk_kuio(&user, buf, size, 0, UIO_WRITE); 	//set up userspace
	
	//write to console
	int result = VOP_WRITE(console_vnode, &user); 
	
	if(result == 1){
		lock_release(cons_lock);
		return EFAULT;
	}
	
	lock_release(cons_lock);
	
	*retval = size - user.uio_resid;
	
	return 0;
	
}

int sys_read(int fd, userptr_t buf, size_t size, int *retval){
	
	if(fd!=STDIN_FILENO){
			return EBADF;
	}
	
	//lock to write/read to/from console
	lock_acquire(cons_lock);
	
	//declare userspace
	struct uio user;		
	struct vnode *console_vnode;
	
	//set up vnode for console
	char *console = kstrdup("con:");
	int ret = vfs_open(console, O_RDONLY, &console_vnode);
	
	//set up userspace
	mk_kuio(&user, buf, size, 0, UIO_READ); 	//set up userspace
	
	//write to console
	int result = VOP_READ(console_vnode, &user); 
	kfree(console);
	
	lock_release(cons_lock);
	
	if(result == 1){
		return EFAULT;
	}
	
	*retval = size - user.uio_resid;
	return 0;
}

int sys_fork(struct trapframe *ptf, pid_t * retval){

	//kprintf("numthreads is %d\n", numthreads);
	
	lock_acquire(fork_lock);
	
	struct addrspace *child_as;
	struct thread *child_thread;
	
	//copy parent trapframe onto heap
	struct trapframe *ctf;
	ctf = (struct trapframe *) kmalloc(sizeof(struct trapframe));
	if(ctf==NULL){
		lock_release(fork_lock);
		return ENOMEM;
	}
	*ctf=*ptf;
	//lock_release(fork_lock);
	
	//copy parent address space
	as_copy(curthread->t_vmspace, &child_as);
	
	lock_release(fork_lock);
	//create new thread
	int ret= thread_fork(curthread->t_name, ctf, (unsigned long) child_as, (void (* )(void *, unsigned long)) md_forkentry, &child_thread);
	if(ret){
		return ENOMEM;
	}
	
	//flushes the tlb??
	//as_activate(curthread->t_vmspace);
	
	//Return child pid to parent (v0 is return value)
	*retval = child_thread->t_pid;
	
	//lock_release(process_lock);
	
	return 0;
}

void 
md_forkentry(struct trapframe *ptf, unsigned long c_as)
{
	/*
	 * This function is provided as a reminder. You need to write
	 * both it and the code that calls it.
	 *
	 * Thus, you can trash it err = sys_forkand do things another way if you prefer.
	 */
	
	//copy the parent's trapframe and address space from heap
	struct trapframe local_tf= *ptf;
	//struct trapframe newtf;
	struct addrspace * local_as = (struct addrspace*) c_as;
	
	//set registers to 0 for return value to child
	local_tf.tf_v0=0;
	local_tf.tf_a3=0;
	
	//increment program counter
	local_tf.tf_epc= local_tf.tf_epc + 4;
	
	//assign copy of address space to curthread and activate it
	curthread->t_vmspace= local_as; 
	as_activate(local_as);
	//kprintf("address space is at %d\n", (int) curthread->t_vmspace);
	
	mips_usermode(&local_tf);
}

int sys_getpid(pid_t * retval){
	*retval=curthread->t_pid;
	return 0;
}

int sys_waitpid(pid_t pid, int *status, int options, pid_t *retval){
	//case where status is null
	if(status==NULL){
		return EFAULT;
	}
	//case where status is not aligned by 4
	if((int)status % 4 !=0){
		return EFAULT;
	}
	//case where option is not supported
	if(options!=0){
		return EINVAL;
	}
	
	//ACQUIRE LOCK TO ACCESS P_TABLE
	lock_acquire(process_lock);
	
	//case where process does not exist
	//waitpid fails
	if(p_table[pid]==NULL){
		lock_release(process_lock);
		return -1;
	}
	//case where the process is not a child of the current process
	//return error
	else if(p_table[pid]->parent_pid!=curthread->t_pid){
		kprintf("thread %d is not a child of thread %d! cannot wait.\n", pid, curthread->t_pid);
		lock_release(process_lock);
		return -1;
	}
	//case where process is a child of the current process and has already exited
	//return immediately
	else if(p_table[pid]->exited==1){
		//kprintf("thread %d is waiting on thread %d.\n", curthread->t_pid, pid);
		*status=p_table[pid]->exitcode;
		lock_release(process_lock);
		*retval=pid;
		return 0;
	}
	//case where the process is a child of the current process and has not yet exited
	//wait until it exits
	else{
		lock_release(process_lock);
		//wait for process to exit
		P(p_table[pid]->exit_sem);
		
		lock_acquire(process_lock);
		//return exitcode to status
		*status=p_table[pid]->exitcode;
		*retval=pid;
		
		//Parent is finished with child process
		//thus we are allowed to free the mallocs for child process block
		kfree(p_table[pid]->exit_sem);
		kfree(p_table[pid]);
		p_table[pid]=NULL;
		
		lock_release(process_lock);
		
		return 0;
	}
}

int sys__exit(int exit_code){
	lock_acquire(process_lock);
	
	//change status in process block
	p_table[curthread->t_pid]->exited=1;
	p_table[curthread->t_pid]->exitcode=exit_code;
	
	//release semaphore so parents can retrieve exitcode
	V(p_table[curthread->t_pid]->exit_sem);
	lock_release(process_lock);
	thread_exit();
	return 0;
}

int
sys_execv(const char *program, char **args){

	struct vnode *v;
	vaddr_t entrypoint, stackptr;
	int result;

	kprintf("the program being run is %s\n", program);

	/* Open the file. */
	result = vfs_open(program, O_RDONLY, &v);
	if (result) {
		return result;
	}

	/* Create a new address space. */
	struct addrspace *new_vmspace = as_create();	
	//struct addrspace *to_delete_vmspace = curthread->t_vmspace;

	if (new_vmspace == NULL) {
		vfs_close(v);
		return ENOMEM;
	}

	if(new_vmspace!= NULL)
	{
		curthread->t_vmspace = new_vmspace;
		//as_destroy(to_delete_vmspace);
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

	int argument_count =0;
	while(((char**)args)[argument_count]!= NULL)
	{
		argument_count++;
	}

	int i;
	int offset = 0;
	vaddr_t string_ptr[argument_count+1];
	
	string_ptr[argument_count] = NULL; 

	/* Iterate through each argument and copy it from kernel to user address */
	for(i = 0; i < argument_count; i++)
	{
		//kprintf("length is %d\n", strlen(arguments[i]) +1);
		offset = offset + strlen(args[i]) + 1; //increment stack offset by string length
		//kprintf("offset is %d\n", offset);
		string_ptr[i] = stackptr - offset; //finding out address of string pointer on the stack
		//kprintf("string pointer is %d\n",string_ptr[i]);
		copyout(args[i], (userptr_t) string_ptr[i], strlen(args[i]) + 1);//copy string arguments from kernel to user address
	}

	int pointers_space = (argument_count + 1) * sizeof(char*);  //calculate how much space is needed for all the string pointers
	//kprintf("pointer space is:%d\n", pointer_space);
	offset = offset + pointers_space; // increase stack 
	//kprintf("offset is %d\n", offset);
	int padding = (stackptr - offset) % 4; // make sure everything is aligned
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

void
mips_syscall(struct trapframe *tf)
{
	int callno;
	int32_t retval;
	int err;

	assert(curspl==0);

	callno = tf->tf_v0;

	/*
	 * Initialize retval to 0. Many of the system calls don't
	 * really return a value, just 0 for success and -1 on
	 * error. Since retval is the value returned on success,
	 * initialize it to 0 by default; thus it's not necessary to
	 * deal with it except for calls that return other values, 
	 * like write.
	 */

	retval = 0;

	switch (callno) {
	    case SYS_reboot:
		err = sys_reboot(tf->tf_a0);
		break;

	    /* Add stuff here */
	    case SYS_write:
			if(cons_lock==NULL){
				cons_lock=lock_create("console lock");
			}
			
			err = sys_write(tf->tf_a0, (userptr_t) tf->tf_a1, tf->tf_a2, &retval);
	    break;
	    case SYS_read:
		    if(cons_lock==NULL){
		    	cons_lock=lock_create("console lock");
		    }
		    err = sys_read(tf->tf_a0, (userptr_t) tf->tf_a1, tf->tf_a2, &retval);
	    break;
	    case SYS_fork:
	    	if(fork_lock==NULL){
				fork_lock=lock_create("fork lock");
			}
	    	err = sys_fork(tf, &retval);
	    break;
	    case SYS_getpid:
	    	err = sys_getpid(&retval);
	    break;
	    case SYS_waitpid:
	    	err = sys_waitpid(tf->tf_a0, (int *) tf->tf_a1, tf->tf_a2, &retval);
	    break;
	    case SYS__exit:
	    	err = sys__exit(tf->tf_a0);
	    break;
	    case SYS_execv:
				err = sys_execv((char *)tf->tf_a0, (char**)tf->tf_a1);	    
	    break;
 
	    default:
		kprintf("Unknown syscall %d\n", callno);
		err = ENOSYS;
		break;
	}


	if (err) {
		/*
		 * Return the error code. This getschild_as converted at
		 * userlevel to a return value of -1 and the error
		 * code in errno.
		 */
		tf->tf_v0 = err;
		tf->tf_a3 = 1;      /* signal an error */
	}
	else {
		/* Success. */
		tf->tf_v0 = retval;
		tf->tf_a3 = 0;      /* signal no error */
	}
	
	/*
	 * Now, advance the program counter, to avoid restarting
	 * the syscall over and over again.
	 */
	
	tf->tf_epc += 4;

	/* Make sure the syscall code didn't forget to lower spl */
	assert(curspl==0);
}

