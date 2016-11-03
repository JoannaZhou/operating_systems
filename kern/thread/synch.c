/*
 * Synchronization primitives.
 * See synch.h for specifications of the functions.
 */

#include <types.h>
#include <lib.h>
#include <synch.h>
#include <thread.h>
#include <curthread.h>
#include <machine/spl.h>

////////////////////////////////////////////////////////////
//
// Semaphore.

struct semaphore *
sem_create(const char *namearg, int initial_count)
{
	struct semaphore *sem;

	assert(initial_count >= 0);

	sem = kmalloc(sizeof(struct semaphore));
	if (sem == NULL) {
		return NULL;
	}

	sem->name = kstrdup(namearg);
	if (sem->name == NULL) {
		kfree(sem);
		return NULL;
	}

	sem->count = initial_count;
	return sem;
}

void
sem_destroy(struct semaphore *sem)
{
	int spl;
	assert(sem != NULL);

	spl = splhigh();
	assert(thread_hassleepers(sem)==0);
	splx(spl);

	/*
	 * Note: while someone could theoretically start sleeping on
	 * the semaphore after the above test but before we free it,
	 * if they're going to do that, they can just as easily wait
	 * a bit and start sleeping on the semaphore after it's been
	 * freed. Consequently, there's not a whole lot of point in 
	 * including the kfrees in the splhigh block, so we don't.
	 */

	kfree(sem->name);
	kfree(sem);
}

void 
P(struct semaphore *sem)
{
	int spl;
	//assert(sem != NULL);
	if(sem == NULL)
	{
		panic("sem %s is NULL at %p \n", sem->name, sem);
	}

	/*
	 * May not block in an interrupt handler.
	 *
	 * For robustness, always check, even if we can actually
	 * complete the P without blocking.
	 */
	assert(in_interrupt==0);

	spl = splhigh();
	while (sem->count==0) {
		thread_sleep(sem);
	}
	assert(sem->count>0);
	sem->count--;
	splx(spl);
}

void
V(struct semaphore *sem)
{
	int spl;
	assert(sem != NULL);
//	if(sem == NULL)
//	{
//		panic("sem %s is NULL at %p \n", sem->name, sem);
//	}
	spl = splhigh();
	sem->count++;
	assert(sem->count>0);
	thread_wakeup(sem);
	splx(spl);
}

////////////////////////////////////////////////////////////
//
// Lock.

struct lock *
lock_create(const char *name)
{
	struct lock *lock;

	lock = kmalloc(sizeof(struct lock));
	if (lock == NULL) {
		return NULL;
	}

	lock->name = kstrdup(name);
	if (lock->name == NULL) {
		kfree(lock);
		return NULL;
	}
	
	lock->lock_holder = NULL;
	
	return lock;
}

void
lock_destroy(struct lock *lock)
{
	assert(lock != NULL);

	// add stuff here as needed
	
	kfree(lock->name);
	kfree(lock);
}

void
lock_acquire(struct lock *lock)
{
	assert(lock != NULL);

	int old_spl; //initial interrupt value
	old_spl = splhigh();//turn off interrupts

	assert(!lock_do_i_hold(lock)); //check for deadlock
//	if(lock_do_i_hold(lock)){
//		kprintf("deadlocking on lock %s\n", lock->name);
//	}

	while (lock->lock_holder != NULL)  //loop until lock is released
	{
		thread_sleep(lock);
	}

	lock->lock_holder = curthread; //grab lock for current thread
	splx(old_spl); //restore interrupt to previous value

}

void
lock_release(struct lock *lock)
{

	assert(lock != NULL);

	int old_spl; //initial interrupt value
	old_spl = splhigh(); //turn off interrupts

	assert(lock->lock_holder == curthread); // lock not held by thread

	lock->lock_holder = NULL; //release lock
	thread_wakeup(lock);
	splx(old_spl); //restore interrupt to previous value
}

int
lock_do_i_hold(struct lock *lock)
{
	assert(lock != NULL);

	int old_spl; //initial interrupt value
	old_spl = splhigh(); //turn off interrupts

	if(lock->lock_holder == curthread) // checks holder
	{
		splx(old_spl); //restore interrupt to previous value
		return 1;
	}
	else
	{
		splx(old_spl); //restore interrupt to previous value
		return 0;
	}

}

////////////////////////////////////////////////////////////
//
// CV


struct cv *
cv_create(const char *name)
{
	struct cv *cv;

	cv = kmalloc(sizeof(struct cv));
	if (cv == NULL) {
		return NULL;
	}

	cv->name = kstrdup(name);
	if (cv->name==NULL) {
		kfree(cv);
		return NULL;
	}
	
	return cv;
}

void
cv_destroy(struct cv *cv)
{
	assert(cv != NULL);
	
	kfree(cv->name);
	kfree(cv);
}

void
cv_wait(struct cv *cv, struct lock *lock)
{
	assert(lock != NULL); 
	assert(cv != NULL);

	int old_spl;
	old_spl = splhigh(); //turn off interrupt

	lock_release(lock); //release the supplied lock
	thread_sleep(cv); //put all threads on CV to sleep 
	thread_wakeup(lock); //wake up lock to CV
	lock_acquire(lock); //re-acquire the lock

	splx(old_spl); //restore interrupt to previous level
	
}

void
cv_signal(struct cv *cv, struct lock *lock)
{
	assert(lock != NULL);
	assert(cv != NULL);

	int old_spl;
	old_spl = splhigh(); //turn off interrupt

	assert(lock_do_i_hold(lock)); //check if we hold the lock to CV
	thread_wakeup_one(cv);//Wake up one thread sleeping on this CV

	splx(old_spl); //restore interrupt to previous level
}

void
cv_broadcast(struct cv *cv, struct lock *lock)
{
	assert(lock != NULL);
	assert(cv != NULL);

	int old_spl;
	old_spl = splhigh(); //turn off interrupt
	
	assert(lock_do_i_hold(lock)); //check if we hold the lock
	thread_wakeup(cv); // wake up all threads sleeping on this CV

	splx(old_spl); //restore interrupt to previous level

}
