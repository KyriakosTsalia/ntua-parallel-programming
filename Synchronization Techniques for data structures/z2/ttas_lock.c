#include "lock.h"
#include "../common/alloc.h"

typedef enum {
        UNLOCKED = 0,
        LOCKED
} lock_state_t;

struct lock_struct {
        lock_state_t state;
};


lock_t *lock_init(int nthreads)
{
	lock_t *lock;

	XMALLOC(lock, 1);
	//printf("%d",lock->state);
	/* other initializations here. */
	return lock;
}

void lock_free(lock_t *lock)
{
	XFREE(lock);
}

void lock_acquire(lock_t *lock)
{
lock_t *l=lock;

	while (1) {
	//	printf("%d",l->state);
		while (l->state==LOCKED){};
		if (__sync_lock_test_and_set(&l->state, LOCKED) ==UNLOCKED)
		return;
}
}

void lock_release(lock_t *lock)
{
        lock_t *l = lock;
	       // __sync_lock_release(&l->state);
		l->state=UNLOCKED;
	//	printf("%d",l->state);
		return;

}
