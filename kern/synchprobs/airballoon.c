/*
 * Driver code for airballoon problem
 */
#include <types.h>
#include <lib.h>
#include <thread.h>
#include <test.h>
#include <synch.h>

#define N_LORD_FLOWERKILLER 8
#define NROPES 16
static int ropes_left = NROPES;

/* Data structures for rope mappings */

/* Implement this! */

volatile int hook_connect[NROPES];
volatile int hooks_on_stakes[NROPES];

volatile int stakes_connect[NROPES];


/* Synchronization primitives */

/* Implement this! */


static struct lock* rope_lk[NROPES];
static struct lock* ropeleft_lk;



static struct lock* balloon_lk;
static struct cv* airballon_cv;
static struct lock* main_thread_lk;
static struct cv* main_thread_cv;



/*
 * Describe your design and any invariants or locking protocols
 * that must be maintained. Explain the exit conditions. How
 * do all threads know when they are done?
 */

static
void
dandelion(void *p, unsigned long arg) //prince
{
	(void)p;
	(void)arg;

	kprintf("Dandelion thread starting\n");

	/* Implement this function */


	while(ropes_left > 0){
	    int rand_hooks = random()%NROPES;  //get the rand hooks to disconnect

	    lock_acquire(rope_lk[rand_hooks]);  //before check this hook situation, lock it

	    
	    while(hook_connect[rand_hooks] == 0 ){   	//if this one equal to zero, means this hook has been disconnected with rope, 
																						//we need to get a new hook to disconnect
	        																			
	        lock_release(rope_lk[rand_hooks]); //check finish, release it


	        rand_hooks = random()%NROPES;   //if we enter this loop, means we need to get a new hook, we choose a wrong hook

	        lock_acquire(rope_lk[rand_hooks]);  //prepare for next check

	    }
		//we still hold the lock of rope_lk[rand_hooks], we need to remember to release it

	    hook_connect[rand_hooks] = 0;      //we get the connected hook, diconnect it

		lock_release(rope_lk[rand_hooks]);


	    lock_acquire(ropeleft_lk);   //get the lock of num of left ropes,
	    ropes_left--;					//less left rope
	    lock_release(ropeleft_lk);   //release the lock of num of left ropes


		kprintf("Dandelion severed rope %d\n", rand_hooks);
	    thread_yield();                //lets go to another thread
		
	}

	
	kprintf("Dandelion thread done\n");



	cv_signal(airballon_cv, balloon_lk);


}










static
void
marigold(void *p, unsigned long arg)
{
	(void)p;
	(void)arg;

	kprintf("Marigold thread starting\n");
	/* Implement this function */


	while(ropes_left > 0){

	    int random_stake = random()%NROPES;     		//choose a rand stakes

	    for(int i = 0; i < NROPES; i++){       			//loop find which hook connects with this stake

	        lock_acquire(rope_lk[i]);					//check before acquire this lock***************************************--->**************************************
																																									//  *
	        if(hooks_on_stakes[i] == random_stake){     //find which hook connect with this stakes																		*
																																									//  *
				if (hook_connect[i] == 1){              //we have already find the corresponding hook, check it still needs disconnect or it was done by D				*
	                    								//this stake is connect with a hook																				*
					hook_connect[i] = 0;                //disconnect it																								  	*
					lock_release(rope_lk[i]);			//release the lock*************************************************<---**************<---************************
																																									//  *
																																									//	*
					lock_acquire(ropeleft_lk);			//get the lock of num of left ropes																				*
					ropes_left--;						//decrease 1																									*
					lock_release(ropeleft_lk);			//release the lock																								*
																																									//	*
																																									//	*
					kprintf("Marigold severed rope %d from stake %d\n", i, random_stake);   //print the rope information and stake information							*
																																									//	*
					thread_yield();                     //go to another thread																							*
					break;								//when come back, move out from this loop, we need to find another appropriate stake. 							*
																																									//	*
				}																																					//	*
				else{																																				//	*
					lock_release(rope_lk[i]);			//release the lock*****************************************************<---************<---**********************
				}																																					//	*
																																									//	*
	        }																																						//	*
			else{																																					//	*
	            lock_release(rope_lk[i]);				//release this lock******************************<---*********************************<---***********************
	        }

        }



	}
	
	kprintf("Marigold thread done\n");

}

static
void
flowerkiller(void *p, unsigned long arg)
{
	(void)p;
	(void)arg;

	kprintf("Lord FlowerKiller thread starting\n");

	/* Implement this function */

	while(ropes_left > 0){
	
	    int from_stake = random()%NROPES;   							//move rope from this stake
	    int to_stake = random()%NROPES;									//move stake to this stake
	    while(from_stake == to_stake){
	        from_stake = random()%NROPES;								//if they are equal, get another one, avoid flower killer sleep
	    }


		// lock_acquire(ropeleft_lk)
		// if(ropes_left == 0){
		//	lock_release(ropeleft_lk)
		// 	break;
		// }
		


	    for(int i = 0; i < NROPES; i++){								//similar with M, loop to find the corresponding hook connect with this stake
	        lock_acquire(rope_lk[i]);									//acquire the lock of this hook[1]
	        
	        if(hooks_on_stakes[i] == from_stake){						//find the corresponding hook connect with this stake

				if(hook_connect[i] == 1){								//check wether this hook is still connect
					hooks_on_stakes[i] = to_stake;						//change the stake
					kprintf("Lord FlowerKiller switched rope %d from stake %d to stake %d\n", i, from_stake, to_stake);
					lock_release(rope_lk[i]); 							//release the lock after change the stake of hook[1]

					//kprintf("Lord FlowerKiller switched rope %d from stake %d to stake %d\n", i, from_stake, to_stake);
					
					thread_yield();  									//change to another thread
	            	break;												//we finish this part job, no need to stay in this loop, go out, get another stake

				}
				else{
					lock_release(rope_lk[i]);							//release the lock after checking[1]
				}
	        }
			else{
	            lock_release(rope_lk[i]);								//release after checking[1]
	        }
        }

		// lock_acquire(ropeleft_lk)
		// if(ropes_left == 0){
		//	lock_release(ropeleft_lk)
		// 	break;
		// }
		
	}
	kprintf("Lord FlowerKiller thread done\n");
}

static
void
balloon(void *p, unsigned long arg)
{
	(void)p;
	(void)arg;

	kprintf("Balloon thread starting\n");

	lock_acquire(balloon_lk);
	cv_wait(airballon_cv, balloon_lk);
	lock_release(balloon_lk);
	kprintf("Balloon freed and Prince Dandelion escapes!\n");
	kprintf("Balloon thread done\n");
	
	cv_signal(main_thread_cv, main_thread_lk);


	/* Implement this function */
}







// Change this function as necessary
int
airballoon(int nargs, char **args)
{

// 	int err = 0, i;

// 	(void)nargs;
// 	(void)args;
// 	(void)ropes_left;


// 	goto done;
// panic:
// 	panic("airballoon: thread_fork failed: %s)\n",
// 	      strerror(err));

// done:
// 	return 0;



	int err = 0, i;

	(void)nargs;
	(void)args;
	(void)ropes_left;
		
	i = 0;

	for(i = 0; i < NROPES; i ++){
	    hook_connect[i] = 1;        //at the beginning all the hooks are connected with ropes
	    hooks_on_stakes[i] = i;      //at the beginning every hooks connect with corresponding stakes
	}

	for(i = 0; i < NROPES; i++){
	    rope_lk[i] = lock_create("rope_lk_" + i);
	}
	ropeleft_lk = lock_create("ropeleft_lk");


	err = thread_fork("Marigold Thread",
			  NULL, marigold, NULL, 0);
	if(err)
		goto panic;
	//........................................................................
	err = thread_fork("Dandelion Thread",
			  NULL, dandelion, NULL, 0);
	if(err)
		goto panic;
	//........................................................................
	for (i = 0; i < N_LORD_FLOWERKILLER; i++) {
		err = thread_fork("Lord FlowerKiller Thread",
				  NULL, flowerkiller, NULL, 0);
		if(err)
			goto panic;
	}
	//........................................................................
	err = thread_fork("Air Balloon",
			  NULL, balloon, NULL, 0);
	if(err)
		goto panic;

	goto done;

panic:
	panic("airballoon: thread_fork failed: %s)\n",
	      strerror(err));
	
done:

	balloon_lk = lock_create("balloon_lk");

	airballon_cv = cv_create("airballon_cv");

	main_thread_lk = lock_create("main_thread_lk");

	main_thread_cv = cv_create("main_thread_cv");



	
    lock_acquire(main_thread_lk);
    cv_wait(main_thread_cv, main_thread_lk);
    lock_release(main_thread_lk);
    


    for(i = 0; i < NROPES; i++){
        lock_destroy(rope_lk[i]);
    }
    lock_destroy(balloon_lk);
	lock_destroy(main_thread_lk);


    cv_destroy(airballon_cv);
    cv_destroy(main_thread_cv);



    kprintf("Main thread done\n");
    return 0;

}
