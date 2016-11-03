/*
 * catlock.c
 *
 * 30-1-2003 : GWA : Stub functions created for CS161 Asst1.
 *
 * NB: Please use LOCKS/CV'S to solve the cat syncronization problem in 
 * this file.
 */


/*
 * 
 * Includes
 *
 */

#include <types.h>
#include <lib.h>
#include <test.h>
#include <thread.h>


/*
 * 
 * Constants
 *
 */

/*
 * Number of food bowls.
 */

#define NFOODBOWLS 2

/*
 * Number of cats.
 */

#define NCATS 6

/*
 * Number of mice.
 */

#define NMICE 2

//Locks
struct status
{
 int iteration;
 int lock;
};

struct lock *bowl1_lock;
struct lock *bowl2_lock;
struct lock *mutex;

//Locks and iterations for both cats and mice all initiated to 0
int bowl1=0;
int bowl2=0;
int cat_iteration[6]={0};
int mouse_iteration[2]={0};
int cat_eating=0;
int mouse_eating=0;

/*
 * 
 * Function Definitions
 * 
 */


/* who should be "cat" or "mouse" */
static void
lock_eat(const char *who, int num, int bowl, int iteration)
{
        kprintf("%s: %d starts eating: bowl %d, iteration %d\n", who, num, 
                bowl, iteration);
        clocksleep(1);
        kprintf("%s: %d ends eating: bowl %d, iteration %d\n", who, num, 
                bowl, iteration);
}

static void eat_food(const char *who, 
					 struct lock *bowl_lock, 
					 int *animal_eating, 
					 int *bowl, 
					 int animalnumber,
					 int bowlnumber,
					 int * iteration){
	lock_acquire(bowl_lock);
	(*animal_eating)++;
	*bowl=1;
	lock_release(mutex);
	lock_eat(who, animalnumber, bowlnumber, *iteration);
	lock_acquire(mutex);
	*bowl=0;
	lock_release(bowl_lock);
	(*animal_eating)--;
	(*iteration)++;
}

/*
 * catlock()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long catnumber: holds the cat identifier from 0 to NCATS -
 *      1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Write and comment this function using locks/cv's.
 *
 */

static
void
catlock(void * unusedpointer, 
        unsigned long catnumber)
{
        /*
         * Avoid unused variable warnings.
         */

        (void) unusedpointer;
        (void) catnumber;
        
        
        while(cat_iteration[catnumber] < 4){
        		
        		lock_acquire(mutex); //Protects between cats and mice 
        		//CHECK FOR MICE
        		if(mouse_eating==0){
        			
        			//CHECK FOR VACANT BOWLS
        			if(bowl1==0 && bowl2==0){ //both bowls vacant
        				int rand=random()%2;
        				if(rand==0){
        					eat_food("cat", bowl1_lock, &cat_eating, &bowl1, catnumber, 1, &cat_iteration[catnumber]);
        				}
        				if(rand==1){
							eat_food("cat", bowl2_lock, &cat_eating, &bowl2, catnumber, 2, &cat_iteration[catnumber]);
        				}
        			}
        			else if(bowl1==0){
    					eat_food("cat", bowl1_lock, &cat_eating, &bowl1, catnumber, 1, &cat_iteration[catnumber]);
        			}
        			else if(bowl2==0){
        				eat_food("cat", bowl2_lock, &cat_eating, &bowl2, catnumber, 2, &cat_iteration[catnumber]);
        			}
        		}
        		lock_release(mutex);
        }
}
	

/*
 * mouselock()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long mousenumber: holds the mouse identifier from 0 to 
 *              NMICE - 1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Write and comment this function using locks/cv's.
 *
 */

static
void
mouselock(void * unusedpointer,
          unsigned long mousenumber)
{
        /*
         * Avoid unused variable warnings.
         */
        
        (void) unusedpointer;
        (void) mousenumber;
        
        while(mouse_iteration[mousenumber] < 4){
				
				lock_acquire(mutex); //Protects between cats and mice 
				
				//CHECK FOR CATS
				if(cat_eating==0){					
					//CHECK FOR VACANT BOWLS
					if(bowl1==0 && bowl2==0){ //both bowls vacant
						int rand=random()%2;
						if(rand==0){
							eat_food("mouse", bowl1_lock, &mouse_eating, &bowl1, mousenumber, 1, &mouse_iteration[mousenumber]);
						}
						if(rand==1){
							eat_food("mouse", bowl2_lock, &mouse_eating, &bowl2, mousenumber, 2, &mouse_iteration[mousenumber]);
						}
					}
					else if(bowl1==0){
						eat_food("mouse", bowl1_lock, &mouse_eating, &bowl1, mousenumber, 1, &mouse_iteration[mousenumber]);
					}
					else if(bowl2==0){
						eat_food("mouse", bowl2_lock, &mouse_eating, &bowl2, mousenumber, 2, &mouse_iteration[mousenumber]);
					}
				}
				lock_release(mutex);
		}
}


/*
 * catmouselock()
 *
 * Arguments:
 *      int nargs: unused.
 *      char ** args: unused.
 *
 * Returns:
 *      0 on success.
 *
 * Notes:
 *      Driver code to start up catlock() and mouselock() threads.  Change
 *      this code as necessary for your solution.
 */

int
catmouselock(int nargs,
             char ** args)
{
        int index, error;
   
        /*
         * Avoid unused variable warnings.
         */
        
        

        (void) nargs;
        (void) args;
        
        bowl1_lock=lock_create("bowl1_lock");
        bowl2_lock=lock_create("bowl2_lock");
        mutex=lock_create("mutex");
        
        /*
         * Start NCATS catlock() threads.
         */

        for (index= 0; index < NCATS; index++) {
           
                error = thread_fork("catlock thread", 
                                    NULL, 
                                    index, 
                                    catlock, 
                                    NULL
                                    );
                
                /*
                 * panic() on error.
                 */

                if (error) {
                 
                        panic("catlock: thread_fork failed: %s\n", 
                              strerror(error)
                              );
                }
        }

        /*
         * Start NMICE mouselock() threads.
         */

        for (index = 0; index < NMICE; index++) {
   
                error = thread_fork("mouselock thread", 
                                    NULL, 
                                    index, 
                                    mouselock, 
                                    NULL
                                    );
      
                /*
                 * panic() on error.
                 */

                if (error) {
         
                        panic("mouselock: thread_fork failed: %s\n", 
                              strerror(error)
                              );
                }
        }

        return 0;
}

/*
 * End of catlock.c
 */
