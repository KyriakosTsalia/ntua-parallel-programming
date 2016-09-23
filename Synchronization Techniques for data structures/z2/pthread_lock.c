#include "lock.h"
#include "../common/alloc.h"
#include <pthread.h>
struct lock_struct {
	/* Delete this in your implementation, just a placeholder. */
	 pthread_spinlock_t state;

};

lock_t *lock_init(int nthreads)
{	
	
	lock_t *lock;
	int pshared;
	XMALLOC(lock, 1);
	/* other initializations here. */
	pthread_spin_init(&lock->state,pshared);
	return lock;
}

void lock_free(lock_t *lock)
{
	XFREE(lock);
}

void lock_acquire(lock_t *lock)
{
pthread_spin_lock(&lock->state);
}

void lock_release(lock_t *lock)
{
pthread_spin_unlock(&lock->state);
}
