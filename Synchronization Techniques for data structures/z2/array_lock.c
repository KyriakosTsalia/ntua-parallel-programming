#include "lock.h"
#include "../common/alloc.h"
#include <string.h>

#define MAX_THREADS 64
struct lock_struct {
	int flag[MAX_THREADS];
	int tail;
	int size;
};

 __thread int slot_index;
 
lock_t *lock_init(int nthreads)
{
	lock_t *lock;

	XMALLOC(lock, 1);
	memset(lock->flag,0,MAX_THREADS);
	lock->flag[0]=1;
	lock->tail=0;
	lock->size=nthreads;
	return lock;
}

void lock_free(lock_t *lock)
{
	XFREE(lock);
}

void lock_acquire(lock_t *lock)
{
	int slot=__sync_fetch_and_add(&lock->tail,1) % lock->size;
	slot_index=slot;
	while(lock->flag[slot]==0) {};
	
}

void lock_release(lock_t *lock)
{
	int slot=slot_index;
//	printf("%d   %d\n",slot_index,lock->tail);
	lock->flag[slot]=0;
	lock->flag[(slot+1) % lock->size]=1;
}
