#ifndef _SYSCALL_H_
#define _SYSCALL_H_

/*
 * Prototypes for IN-KERNEL entry points for system call implementations.
 */

int sys_reboot(int code);
int sys_write(int fd, userptr_t buf, size_t size, int *retval);
int sys_read(int fd, userptr_t buf, size_t size, int *retval);
int sys_fork(struct trapframe *ptf, pid_t * retval);
int sys_getpid(pid_t * retval);
int sys_waitpid(pid_t pid, int *status, int options, pid_t * retval);
int sys__exit(int exit_code);
int sys_execv(const char *program, char **args); 



#endif /* _SYSCALL_H_ */
