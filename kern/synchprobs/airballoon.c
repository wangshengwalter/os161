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

volatile int hook_connect[NROPES];   		//1 means connect
											//0 means non ropes connect with hook
volatile int hooks_on_stakes[NROPES];		//hooks_on_stakes[4] = 5 means hook 4 connect with stake 5



/* Synchronization primitives */

/* Implement this! */


static struct lock* rope_lk[NROPES];
static struct lock* ropeleft_lk;

static struct lock* dandelion_lk;
static struct cv* dandelion_cv;
static struct lock* marigold_lk;
static struct cv* marigold_cv;
// static struct lock* flowerkiller_lk;
// static struct cv* flowerkiller_cv;





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
		kprintf("Dandelion severed rope %d\n", rand_hooks);

		lock_release(rope_lk[rand_hooks]);


	    lock_acquire(ropeleft_lk);   //get the lock of num of left ropes,
	    ropes_left--;					//less left rope
	    lock_release(ropeleft_lk);   //release the lock of num of left ropes



	    thread_yield();                //lets go to another thread
		
	}

	
	kprintf("Dandelion thread done\n");



	cv_signal(dandelion_cv, dandelion_lk);		//airballoon finish, wakeup it, let it start again


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
					kprintf("Marigold severed rope %d from stake %d\n", i, random_stake);   //print the rope information and stake information							*
					lock_release(rope_lk[i]);			//release the lock*************************************************<---**************<---************************
																																									//  *
																																									//	*
					lock_acquire(ropeleft_lk);			//get the lock of num of left ropes																				*
					ropes_left--;						//decrease 1																									*
					lock_release(ropeleft_lk);			//release the lock																								*
																																									//	*
																																									//	*
					
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
	cv_signal(marigold_cv, marigold_lk);

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

	//craete locks for each person
	dandelion_lk = lock_create("dandelion_lk");         	//create the dandelion lock for dandelion_cv
	dandelion_cv = cv_create("dandelion_cv");				//create the  dandelion_cv to transfer signal
	marigold_lk = lock_create("marigold_lk");
	marigold_cv = cv_create("marigold_cv");
	// flowerkiller_lk = lock_create("flowerkiller_lk");
	// flowerkiller_cv = cv_create("flowerkiller_cv");

	lock_acquire(dandelion_lk);								//acquire the lock of 	dandelion_lk				
	cv_wait(dandelion_cv, dandelion_lk);					//wait the dandelion finish--- Balloon freed
	lock_release(dandelion_lk);

	lock_acquire(marigold_lk);								//acquire the lock of 	marigold_lk				
	cv_wait(marigold_cv, marigold_lk);						//wait the marigold finish--- Balloon freed
	lock_release(marigold_lk);

	// lock_acquire(flowerkiller_lk);							//acquire the lock of 	flowerkiller_lk				
	// cv_wait(flowerkiller_cv, flowerkiller_lk);				//wait the flowerkiller finish--- Balloon freed
	// lock_release(flowerkiller_lk);	

	//delete locks for each person
	lock_destroy(dandelion_lk);								//finsh dandelion thread, no need dandelion_lk
    cv_destroy(dandelion_cv);								//delete dandelion_cv
	lock_destroy(marigold_lk);
	cv_destroy(marigold_cv);
	// lock_destroy(flowerkiller_lk);
	// cv_destroy(flowerkiller_cv);


	kprintf("Balloon freed and Prince Dandelion escapes!\n");
	kprintf("Balloon thread done\n");
	
	cv_signal(main_thread_cv, main_thread_lk);  		//main thread finish, wakeup it, let it start again


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
	    hook_connect[i] = 1;        							//at the beginning all the hooks are connected with ropes
	    hooks_on_stakes[i] = i;      							//at the beginning every hooks connect with corresponding stakes
	}

	for(i = 0; i < NROPES; i++){ 
	    rope_lk[i] = lock_create("rope_lk_" + i);            	//create the lock of each hook and ropes
	}
	ropeleft_lk = lock_create("ropeleft_lk");					//create the lock of num of left ropes


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

	

	main_thread_lk = lock_create("main_thread_lk");		//create the main thread lock for main_thread_cv
	main_thread_cv = cv_create("main_thread_cv");		//create the main_thread_cv to transfer signal back 


    lock_acquire(main_thread_lk);						//lock main_thread_lk
    cv_wait(main_thread_cv, main_thread_lk);			//wait for the main thread finish
    lock_release(main_thread_lk);						//release the main_thread_lk


	lock_destroy(main_thread_lk);						//finsh main thread, no nedd main_thread_lk
    cv_destroy(main_thread_cv);							//delete main_thread_cv

    


    for(i = 0; i < NROPES; i++){              			//destory the rope_lk[0~15]
        lock_destroy(rope_lk[i]);
    }

	lock_destroy(ropeleft_lk);							//destory ropeleft_lk


	ropes_left = NROPES;								//prepare for next run
    

    kprintf("Main thread done\n");
    return 0;

}
