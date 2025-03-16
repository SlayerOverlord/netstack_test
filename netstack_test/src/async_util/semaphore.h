#pragma once

#include <time.h>
#include <threads.h>

/*
*	semaphore_t is a structure for using the semaphore concept.
* 
*	This semaphore requires the external locking/unlocking of
*   the mutex contained within, while not explicitly how
*	a semaphore works, it's useful for handling concurrency.
*	
*	@param S: size_t: the current value of the semaphore
*	@param mtx: mtx_t: a mutex for ensuring mutual exclusion
*					   while changing S value.
*	@param delay: timespec: a delay for waiting on the semaphore
*							value, if 0 no delay will be taken.
*/
typedef struct semaphore
{
	size_t S;
	mtx_t  mtx;
	timespec delay;
} semaphore_t;

int sem_init(semaphore_t* sem, size_t S, timespec delay);
int sem_init_def(semaphore_t* sem);
int sem_destroy(semaphore_t* sem);

int sem_produce(semaphore_t* sem);
int sem_consume(semaphore_t* sem);