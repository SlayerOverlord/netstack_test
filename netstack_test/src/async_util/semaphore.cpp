#include "semaphore.h"

int  _sem_valid(semaphore_t* sem);
void _sem_timeout(semaphore_t* sem);

int sem_init(semaphore_t* sem, size_t S, timespec delay)
{
	if (!_sem_valid(sem)) return thrd_error;

	sem->S = S;
	sem->delay = delay;

	return mtx_init(&sem->mtx, mtx_plain);
}

int sem_init_def(semaphore_t* sem)
{
	return sem_init(sem, 0, { 0 });
}

int sem_destroy(semaphore_t* sem)
{
	int ret = 0;
	if (!_sem_valid(sem)) return thrd_error;

	sem->S = 0;
	sem->delay.tv_sec  = 0;
	sem->delay.tv_nsec = 0;
	mtx_destroy(&sem->mtx);

	return thrd_success;
}

int sem_produce(semaphore_t* sem)
{
	if (!_sem_valid(sem)) return thrd_error;
	if (mtx_lock(&sem->mtx) == thrd_error) return thrd_error;

	//--<Shared Section>--
	sem->S++;
	//--<Shared Section>--

	return mtx_unlock(&sem->mtx);
}

int sem_consume(semaphore_t* sem)
{
	int status = thrd_busy;
	if (!_sem_valid(sem)) return thrd_error;

	while (status == thrd_busy)
	{
		if (mtx_lock(&sem->mtx) == thrd_error) return thrd_error;

		// --<Shared Section>--
		if (sem->S)
		{
			sem->S --;
			status = thrd_success;
		}
		// --<Shared Section>--

		mtx_unlock(&sem->mtx);
		if (status == thrd_busy) _sem_timeout(sem);
	}

	return status;
}

int _sem_valid(semaphore_t* sem)
{
	return sem != NULL;
}

void _sem_timeout(semaphore_t* sem)
{
	if (sem->delay.tv_nsec || sem->delay.tv_sec)
		thrd_sleep(&sem->delay, NULL);
}
