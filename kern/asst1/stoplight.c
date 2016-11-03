/* 
 * stoplight.c
 *
 * 31-1-2003 : GWA : Stub functions created for CS161 Asst1.
 *
 * NB: You can use any synchronization primitives available to solve
 * the stoplight problem in this file.
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
#include <queue.h>


/*
 *
 * Constants
 *
 */

/*
 * Number of cars created.
 */

#define NCARS 20


/*
 *
 * Function Definitions
 *
 */

//SHARED DATA
//semaphores
struct semaphore *numcars_sem;
struct semaphore *q0, *q1, *q2, *q3; 
struct semaphore *access_r10, *access_r11, *access_r12, *access_r13;

int region[4]={0,0,0,0}; 			//indicates vacancy of each region
int num_cars=0;			  			//number of cars in the intersection
struct queue *lane0, *lane1, *lane2, *lane3; //queues to line up cars before they enter intersection
//int read_count0=0; 
//int read_count1=0;
//int read_count2=0;
//int read_count3=0; //allow multiple readers to read queues
int car_route[20][5];				//routing table for all cars

//END SHARED DATA

static const char *directions[] = { "N", "E", "S", "W" };

static const char *msgs[] = {
        "approaching:",
        "region1:    ",
        "region2:    ",
        "region3:    ",
        "leaving:    "
};

/* use these constants for the first parameter of message */
enum { APPROACHING, REGION1, REGION2, REGION3, LEAVING };

static void
message(int msg_nr, int carnumber, int cardirection, int destdirection)
{
        kprintf("%s car = %2d, direction = %s, destination = %s\n",
		msgs[msg_nr], carnumber,
		directions[cardirection], directions[destdirection]);
}
 

static void router(int carnumber, int cardirection, int turndirection, int destdirection){
	
	//update the routing table with initial direction
	car_route[carnumber][0]=cardirection;
	
	//go straight
	if(turndirection==2){
		car_route[carnumber][1]=cardirection+10;
		car_route[carnumber][2]=((cardirection+3)%4) + 10;
		car_route[carnumber][3]=destdirection;
	}
	//turn right
	if(turndirection==3){
		car_route[carnumber][1]=cardirection+10;
		car_route[carnumber][2]=destdirection;
	}
	//turn left
	if(turndirection==1){
		car_route[carnumber][1]=cardirection+10;
		car_route[carnumber][2]=((cardirection+3)%4) + 10;
		car_route[carnumber][3]=((cardirection+2)%4) + 10;
		car_route[carnumber][4]=destdirection;
		
	}
}
/*
 * gostraight()
 *
 * Arguments:
 *      unsigned long cardirection: the direction from which the car
 *              approaches the intersection.
 *      unsigned long carnumber: the car id number for printing purposes.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      This function should implement passing straight through the
 *      intersection from any direction.
 *      Write and comment this function.
 */
static
void
gostraight(unsigned long cardirection,
           unsigned long carnumber,
           int destdirection){

}

//
static
void
gostraightorleft(unsigned long cardirection,
           unsigned long carnumber,
           int destdirection,
           struct queue* lane_q,
           struct semaphore * q_sem)
{
        /*
         * Avoid unused variable warnings.
         */
        
        (void) cardirection;
        (void) carnumber;
        struct semaphore *region_sem;
        int previous_region=cardirection;
        
        int i=0;
        while(car_route[carnumber][i]!= destdirection){
        	//CASE WHERE WE ARE LEAVING INTERSECTION
        	if(car_route[carnumber][i+1]==destdirection){
        		message(4, carnumber, cardirection, destdirection);
        		DEBUG(DB_THREADS, "car %d trying to acquire region_sem\n", carnumber);
        		P(region_sem);
        		DEBUG(DB_THREADS, "car %d acquired region_sem\n", carnumber);
        		//DEBUG(DB_THREADS, "clearing out region %d at ln 156\n", car_route[carnumber][i]-10);
        		region[car_route[carnumber][i]-10] = 0;
        		V(region_sem);
        		DEBUG(DB_THREADS, "car %d released region_sem\n", carnumber);
        		DEBUG(DB_THREADS, "car %d trying to acquire numcars_sem\n", carnumber);
				P(numcars_sem);
				DEBUG(DB_THREADS, "car %d acquired numcars_sem on line 151\n", carnumber);
				num_cars--;
				V(numcars_sem);
				DEBUG(DB_THREADS, "car %d released numcars_sem\n", carnumber);
        		i++;
        	}
        	//CASE WHERE WE ARE INSIDE INTERSECTION
        	else if(i>0){
				//MOVE INTO NEXT REGION
				if(car_route[carnumber][i+1]==10){
					DEBUG(DB_THREADS, "car %d trying to acquire r10\n", carnumber);
					P(access_r10);
					DEBUG(DB_THREADS, "car %d acquired r10\n", carnumber);
					if(region[0]==0){
						region[0] = 1;
						i++;
						message(i, carnumber, cardirection, destdirection);
						region[car_route[carnumber][i-1]-10] = 0;
						region_sem=access_r10;
					}
					V(access_r10);
					DEBUG(DB_THREADS, "car %d released r10\n", carnumber);
				}
				else if(car_route[carnumber][i+1]==11){
					DEBUG(DB_THREADS, "car %d trying to acquire r11\n", carnumber);
					P(access_r11);
					DEBUG(DB_THREADS, "car %d acquired r11\n", carnumber);
					if(region[1]==0){
						region[1] = 1;
						i++;
						message(i, carnumber, cardirection, destdirection);	
						region[car_route[carnumber][i-1]-10] = 0;
						region_sem=access_r11;
					}
					V(access_r11);
					DEBUG(DB_THREADS, "car %d released r10\n", carnumber);
				}
				else if(car_route[carnumber][i+1]==13){ 
					DEBUG(DB_THREADS, "car %d trying to acquire r13\n", carnumber);
					P(access_r13);
					DEBUG(DB_THREADS, "car %d acquired r13\n", carnumber);
					if(region[3]==0){
						region[3] = 1;
						i++;
						message(i, carnumber, cardirection, destdirection);
						region[car_route[carnumber][i-1]-10] = 0;
						region_sem=access_r13;
					}
					V(access_r13);
					DEBUG(DB_THREADS, "car %d released r10\n", carnumber);
				}
				else if(car_route[carnumber][i+1]==12){
					DEBUG(DB_THREADS, "car %d trying to acquire r12\n", carnumber);
					P(access_r12);
					DEBUG(DB_THREADS, "car %d acquired r12\n", carnumber);
					if(region[2]==0){
						region[2] = 1;
						i++;
						message(i, carnumber, cardirection, destdirection);
						region[car_route[carnumber][i-1]-10] = 0;
						region_sem=access_r12;
					}
					V(access_r12);
					DEBUG(DB_THREADS, "car %d released r10\n", carnumber);
				}
        	}
        	//CASE WHERE WE ARE APPROACHING INTERSECTION
        	else {
        		DEBUG(DB_THREADS, "car %d trying to acquire numcars_sem\n", carnumber);
        		P(numcars_sem);
        		DEBUG(DB_THREADS, "car %d acquired numcars_sem on line 212\n", carnumber);
        		//CHECK NUMBER OF CARS
        		if(num_cars < 3){
        			DEBUG(DB_THREADS, "number of cars in intersection is %d\n", num_cars);
					//CHECK FIRST CAR
					P(q_sem);
					//DEBUG(DB_THREADS, "car %d acquired its queue on line 217\n", carnumber);
					int *firstcar=q_getguy(lane_q, q_getstart(lane_q));
					V(q_sem);
					//DEBUG(DB_THREADS, "the first car is carnumber: %d\n", *firstcar);
				
					//IF WE ARE NOT IN INTERSECTION AND WE ARE THE FIRST CAR
					if(i==0 && carnumber==(*firstcar)){
						//MOVE INTO NEXT REGION
						//DEBUG(DB_THREADS, "trying to get into region %d \n", car_route[carnumber][i+1]);
						if(car_route[carnumber][i+1]==10){
							DEBUG(DB_THREADS, "car %d trying to acquire access_r10 on line 228\n", carnumber);
							P(access_r10);
							DEBUG(DB_THREADS, "car %d acquired access_r10 on line 254\n", carnumber);
							if(region[0]==0){
								region[0] = 1;
								//region[car_route[carnumber][i]-10] = 0;
								P(q_sem); 
								struct queue* pop = q_remhead(lane_q); //pop the car out of the lane's queue
								V(q_sem);
								region_sem=access_r10;
								num_cars++;
								i++;
								message(i, carnumber, cardirection, destdirection);
							}
							V(access_r10);
							DEBUG(DB_THREADS, "car %d released r10\n", carnumber);
						}
						else if(car_route[carnumber][i+1]==11){
							DEBUG(DB_THREADS, "car %d trying to acquire access_r11 on line 244\n", carnumber);
							P(access_r11);
							DEBUG(DB_THREADS, "car %d acquired access_r11 on line 246\n", carnumber);
							if(region[1]==0){
								region[1] = 1;
								//region[car_route[carnumber][i]-10] = 0;
								P(q_sem); 
								struct queue* pop = q_remhead(lane_q); //pop the car out of the lane's queue
								V(q_sem);
								region_sem=access_r11;
								num_cars++;
								i++;
								message(i, carnumber, cardirection, destdirection);
							}
							V(access_r11);
							DEBUG(DB_THREADS, "car %d released r11\n", carnumber);
						}
						else if(car_route[carnumber][i+1]==13){
							DEBUG(DB_THREADS, "car %d trying to acquire access_r13 on line 259\n", carnumber);
							P(access_r13);
							DEBUG(DB_THREADS, "car %d acquired access_r13 on line 262\n", carnumber);
							if(region[3]==0){
								region[3] = 1;
								//region[car_route[carnumber][i]-10] = 0;
								P(q_sem); 
								struct queue* pop = q_remhead(lane_q); //pop the car out of the lane's queue
								V(q_sem);
								region_sem=access_r13;
								num_cars++;
								i++;
								message(i, carnumber, cardirection, destdirection);
							}
							V(access_r13);
							DEBUG(DB_THREADS, "car %d released r10\n", carnumber);
						}
						else if(car_route[carnumber][i+1]==12){ 
							DEBUG(DB_THREADS, "car %d trying to acquire access_r12 on line 274\n", carnumber);
							P(access_r12);
							DEBUG(DB_THREADS, "car %d acquired access_r12 on line 278\n", carnumber);
							if(region[2]==0){
								region[2] = 1;
								region[car_route[carnumber][i]-10] = 0;
								P(q_sem); 
								struct queue* pop = q_remhead(lane_q); //pop the car out of the lane's queue
								V(q_sem);
								region_sem=access_r12;
								num_cars++;
								i++;
								message(i, carnumber, cardirection, destdirection);
							}
							V(access_r12);
							DEBUG(DB_THREADS, "car %d released r12\n", carnumber);
						}
					}
        		}
        		V(numcars_sem);
        		DEBUG(DB_THREADS, "car %d released numcars_sem\n", carnumber);
        	}
        }
}


/*
 * turnleft()
 *
 * Arguments:
 *      unsigned long cardirection: the direction from which the car
 *              approaches the intersection.
 *      unsigned long carnumber: the car id number for printing purposes.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      This function should implement making a left turn through the 
 *      intersection from any direction.
 *      Write and comment this function.
 */

static
void
turnleft(unsigned long cardirection,
         unsigned long carnumber,
         int destdirection,
         struct queue* lane_q,
         struct semaphore* q_sem)
{
        /*
         * Avoid unused variable warnings.
         */

        (void) cardirection;
        (void) carnumber;
        
        
}


/*
 * turnright()
 *
 * Arguments:
 *      unsigned long cardirection: the direction from which the car
 *              approaches the intersection.
 *      unsigned long carnumber: the car id number for printing purposes.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      This function should implement making a right turn through the 
 *      intersection from any direction.
 *      Write and comment this function.
 */

static
void
turnright(unsigned long cardirection,
          unsigned long carnumber,
          int destdirection,
          struct queue* lane_q,
          struct semaphore* q_sem)
{
        /*
         * Avoid unused variable warnings.
         */

        (void) cardirection;
        (void) carnumber;
        struct semaphore *region_sem;
        
        
        int i=0;
        while(car_route[carnumber][i]!=destdirection && i < 4){
        	//CASE WHERE WE ARE LEAVING
        	if(car_route[carnumber][i+1]==destdirection){
        		message(4, carnumber, cardirection, destdirection);
				DEBUG(DB_THREADS, "car %d released numcars_sem\n", carnumber);
				DEBUG(DB_THREADS, "clearing out region %d at ln 355\n", car_route[carnumber][i]-10);
				P(region_sem);
				region[car_route[carnumber][i]-10] = 0;
				V(region_sem);
				DEBUG(DB_THREADS, "car %d released region_sem\n", carnumber);
				DEBUG(DB_THREADS, "car %d trying to acquire numcars_sem on line 373\n", carnumber);
				P(numcars_sem);
				DEBUG(DB_THREADS, "car %d acquired numcars_sem on line 375\n", carnumber);
				num_cars--;
				V(numcars_sem);
				i++;
			}
        	//CASE WHERE WE ARE APPROACHING INTERSECTION
        	else {
				//CHECK FIRST CAR
				P(q_sem);
				int *firstcar=q_getguy(lane_q, q_getstart(lane_q));
				V(q_sem);
				//DEBUG(DB_THREADS, "the first car is carnumber: %d\n", *firstcar);
				
				//IF WE ARE NOT IN INTERSECTION AND WE ARE THE FIRST CAR
				if(i==0 && carnumber==(*firstcar)){
					//MOVE INTO NEXT REGION
					//DEBUG(DB_THREADS, "trying to get into region %d \n", car_route[carnumber][i+1]);
					if(car_route[carnumber][i+1]==10){ 
						DEBUG(DB_THREADS, "car %d trying to acquire r10 on right turn\n", carnumber);
						P(access_r10);
						DEBUG(DB_THREADS, "car %d acquired r10 on right turn\n", carnumber);
						if(region[0]==0){
							//DEBUG(DB_THREADS, "trying to acquire r10\n");
							//DEBUG(DB_THREADS, "acquired region 10\n");
							region[0] = 1;
							V(access_r10);
							DEBUG(DB_THREADS, "car %d released r10\n", carnumber);
							P(q_sem); 
							struct queue* pop = q_remhead(lane_q); //pop the car out of the lane's queue
							V(q_sem);
							region_sem=access_r10;
							DEBUG(DB_THREADS, "car %d trying to acquire numcars_sem\n", carnumber);
							P(numcars_sem);
							DEBUG(DB_THREADS, "car %d acquired numcars_sem on line 370\n", carnumber);
							num_cars++;
							V(numcars_sem);
							DEBUG(DB_THREADS, "car %d released r10\n", carnumber);
							i++;
							message(i, carnumber, cardirection, destdirection);
						}
						else{
							V(access_r10);
							DEBUG(DB_THREADS, "car %d released r10\n", carnumber);
						}
					}
					else if(car_route[carnumber][i+1]==11){
						DEBUG(DB_THREADS, "car %d trying to acquire r11 on right turn\n", carnumber);
						P(access_r11);
						DEBUG(DB_THREADS, "car %d acquired r11 on right turn\n", carnumber);
						if(region[1]==0){
							//DEBUG(DB_THREADS, "trying to acquire r11\n");
							//DEBUG(DB_THREADS, "acquired region r11\n");
							region[1] = 1;
							V(access_r11);
							DEBUG(DB_THREADS, "car %d released r11\n", carnumber);
							P(q_sem); 
							struct queue* pop = q_remhead(lane_q); //pop the car out of the lane's queue
							V(q_sem);
							region_sem=access_r11;
							DEBUG(DB_THREADS, "car %d acquired r10\n", carnumber);
							P(numcars_sem);
							DEBUG(DB_THREADS, "car %d acquired numcars_sem line 385\n", carnumber);
							num_cars++;
							V(numcars_sem);
							i++;
							message(i, carnumber, cardirection, destdirection);
						}
						else{
							V(access_r11);
							DEBUG(DB_THREADS, "car %d released r11\n", carnumber);
						}
					}
					else if(car_route[carnumber][i+1]==13){
						DEBUG(DB_THREADS, "car %d trying to acquire r13 on right turn\n", carnumber);
						P(access_r13);
						DEBUG(DB_THREADS, "car %d acquired r11 on right turn\n", carnumber);
						if(region[3]==0){
							region[3] = 1;
							V(access_r13);
							DEBUG(DB_THREADS, "car %d released r13\n", carnumber);
							P(q_sem); 
							struct queue* pop = q_remhead(lane_q); //pop the car out of the lane's queue
							V(q_sem);
							region_sem=access_r13;
							P(numcars_sem);
							DEBUG(DB_THREADS, "car %d acquired numcars_sem on line 477\n", carnumber);
							num_cars++;
							V(numcars_sem);
							DEBUG(DB_THREADS, "car %d released numcars_sem on line 480\n", carnumber);
							i++;
							message(i, carnumber, cardirection, destdirection);
						}
						else{
							V(access_r13);
							DEBUG(DB_THREADS, "car %d released r13\n", carnumber);
						}
					}
					else if(car_route[carnumber][i+1]==12){
						DEBUG(DB_THREADS, "car %d trying to acquire r12 on right turn\n", carnumber);
						P(access_r12);
						DEBUG(DB_THREADS, "car %d acquired r11 on right turn\n", carnumber);
						if(region[2]==0){
							region[2] = 1;
							V(access_r12);
							DEBUG(DB_THREADS, "car %d released r12\n", carnumber);
							P(q_sem); 
							struct queue* pop = q_remhead(lane_q); //pop the car out of the lane's queue
							V(q_sem);
							region_sem=access_r12;
							P(numcars_sem);
							DEBUG(DB_THREADS, "car %d acquired numcars_sem on line 497\n", carnumber);
							num_cars++;
							V(numcars_sem);
							DEBUG(DB_THREADS, "car %d released numcars_sem on line 500\n", carnumber);
							i++;
							message(i, carnumber, cardirection, destdirection);
						}
						else{
							V(access_r12);
							DEBUG(DB_THREADS, "car %d released r12\n", carnumber);
						}
					}
				}
			}
        }
}


/*
 * approachintersection()
 *
 * Arguments: 
 *      void * unusedpointer: currently unused.
 *      unsigned long carnumber: holds car id number.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Change this function as necessary to implement your solution. These
 *      threads are created by createcars().  Each one must choose a direction
 *      randomly, approach the intersection, choose a turn randomly, and then
 *      complete that turn.  The code to choose a direction randomly is
 *      provided, the rest is left to you to implement.  Making a turn
 *      or going straight should be done by calling one of the functions
 *      above.
 */
 
static
void
approachintersection(void * unusedpointer,
                     unsigned long carnumber)
{
        int cardirection, turndirection, destdirection;

        /*
         * Avoid unused variable and function warnings.
         */

        (void) unusedpointer;
        (void) carnumber;
		(void) gostraight;
		(void) turnleft;
		(void) turnright;
		struct queue *q;
		struct semaphore *q_sem;

        /*
         * cardirection is set randomly.
         */

        cardirection = random() % 4;
        turndirection = (random() % 3)+1; //straight =2; right=3; left=1;
        destdirection = (cardirection + turndirection)%4; //maps car's destination depending on turndirection
        if(cardirection==0){
        	q_sem=q0;
        	q=lane0;
        	P(q0);
        	assert(q_addtail(lane0, &carnumber)==0);
        	message(0, carnumber, cardirection, destdirection); //APPROACH
        	V(q0);
        }
        else if(cardirection==1){
        	q_sem=q1;
        	q=lane1;
        	P(q1);
        	assert(q_addtail(lane1, &carnumber)==0);
			message(0, carnumber, cardirection, destdirection); //APPROACH
			V(q1);
		}
        else if(cardirection==2){
        	q_sem=q2;
        	q=lane2;
			P(q2);
			assert(q_addtail(lane2, &carnumber)==0);
			message(0, carnumber, cardirection, destdirection); //APPROACH
			V(q2);
		}
        else if(cardirection==3){
        	q_sem=q3;
        	q=lane3;
			P(q3);
			assert(q_addtail(lane3, &carnumber)==0);
			message(0, carnumber, cardirection, destdirection); //APPROACH
			V(q3);
		}

        router(carnumber, cardirection, turndirection, destdirection);
        
        if(turndirection==2 || turndirection==1){
        	gostraightorleft(cardirection, carnumber, destdirection, q, q_sem);
        }
        else if(turndirection==3){
        	turnright(cardirection, carnumber, destdirection, q, q_sem);
        }
        
        
}


/*
 * createcars()
 *
 * Arguments:
 *      int nargs: unused.
 *      char ** args: unused.
 *
 * Returns:
 *      0 on success.
 *
 * Notes:
 *      Driver code to start up the approachintersection() threads.  You are
 *      free to modiy this code as necessary for your solution.
 */

int
createcars(int nargs,
           char ** args)
{
        int index, error;

        /*
         * Avoid unused variable warnings.
         */

        (void) nargs;
        (void) args;
        
//        r10=sem_create("r10", 1); //region 2
//        r11=sem_create("r11", 1); //region 1
//        r13=sem_create("r13", 1); //region 3
//        r12=sem_create("r12", 1); //region 4
        numcars_sem=sem_create("numcars_sem", 1);
        access_r10=sem_create("access_r10", 1); //region 2
        access_r11=sem_create("access_r11", 1); //region 1
        access_r13=sem_create("access_r13", 1); //region 3
        access_r12=sem_create("access_r12", 1); //region 4
        
        lane0=q_create(20);	//North lane
        lane1=q_create(20);	//East lane
        lane2=q_create(20); //South lane
        lane3=q_create(20); //West lane
        
        q0=sem_create("q0", 1);
        q1=sem_create("q1", 1);
        q2=sem_create("q2", 1);
        q3=sem_create("q3", 1);

        /*
         * Start NCARS approachintersection() threads.
         */

        for (index = 0; index < NCARS; index++) {

                error = thread_fork("approachintersection thread",
                                    NULL,
                                    index,
                                    approachintersection,
                                    NULL
                                    );

                /*
                 * panic() on error.
                 */

                if (error) {
                        
                        panic("approachintersection: thread_fork failed: %s\n",
                              strerror(error)
                              );
                }
        }

        return 0;
}
