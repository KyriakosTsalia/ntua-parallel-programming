#include <stdio.h>
#include <stdlib.h> /* rand() */
#include <limits.h>
#include <pthread.h> /* for pthread_spinlock_t */

#include "../common/alloc.h"
#include "ll.h"

typedef struct ll_node {
	int key;
	pthread_spinlock_t state;
	struct ll_node *next;

	/* other fields here? */
} ll_node_t;

struct linked_list {
	ll_node_t *head;
	/* other fields here? */
};

/**
 * Create a new linked list node.
 **/
static ll_node_t *ll_node_new(int key)
{
	ll_node_t *ret;
	XMALLOC(ret, 1);
	ret->key = key;
	ret->next = NULL;
	/* Other initializations here? */
	int pshared;
	pthread_spin_init(&ret->state,pshared);
	return ret;
}

/**
 * Free a linked list node.
 **/
static void ll_node_free(ll_node_t *ll_node)
{
	XFREE(ll_node);
}

/**
 * Create a new empty linked list.
 **/
ll_t *ll_new()
{
	ll_t *ret;

	XMALLOC(ret, 1);
	ret->head = ll_node_new(-1);
	ret->head->next = ll_node_new(INT_MAX);
	ret->head->next->next = NULL;

	return ret;
}

/**
 * Free a linked list and all its contained nodes.
 **/
void ll_free(ll_t *ll)
{
	ll_node_t *next, *curr = ll->head;
	while (curr) {
		next = curr->next;
		ll_node_free(curr);
		curr = next;
	}
	XFREE(ll);
}

int ll_contains(ll_t *ll, int key)
{
//	printf("Contains \n");
	int ret = 0;
	ll_node_t *curr, *next;
	pthread_spin_lock(&ll->head->state);
	
	curr = ll->head;	
	next = curr->next;
	pthread_spin_lock(&next->state);
	
	while (next->key < key) {
		pthread_spin_unlock(&curr->state);
		curr = next;
		next = curr->next;
		
		pthread_spin_lock(&next->state);
	}
	
	ret = (key == next->key);

//	ll_print(ll);
//	printf("\n %d %d \n",key,ret);
	pthread_spin_unlock(&next->state);
	pthread_spin_unlock(&curr->state);
	
	//printf("*******");
	return ret;
	
	
}

int ll_add(ll_t *ll, int key)
{
	
	int ret = 0;
	ll_node_t *curr, *next;
	ll_node_t *new_node;
	pthread_spin_lock(&ll->head->state);
	curr = ll->head;
	next = curr->next;
	pthread_spin_lock(&next->state);

	while (next->key < key) {
		pthread_spin_unlock(&curr->state);
		curr = next;
		next = curr->next;
		
		pthread_spin_lock(&next->state);
	}

	if (key != next->key) {
		ret = 1;
		new_node = ll_node_new(key);
		new_node->next = next;
		curr->next = new_node;
	}
	
	
	pthread_spin_unlock(&next->state);
	pthread_spin_unlock(&curr->state);
	
	return ret;
	
	
}

int ll_remove(ll_t *ll, int key)
{
//	printf("Remove\n");
	int ret = 0;
	ll_node_t *curr, *next;
	pthread_spin_lock(&ll->head->state);
	
	curr = ll->head;	
	next = curr->next;
	pthread_spin_lock(&next->state);
	
	while (next->key < key) {
		pthread_spin_unlock(&curr->state);
		curr = next;
		next = curr->next;
		
		pthread_spin_lock(&next->state);
	}

	if (key == next->key) {
		ret = 1;
		curr->next = next->next;
		ll_node_free(next);
	//	ll_print(ll);
	}
if (ret ==0 )pthread_spin_unlock(&next->state);	
	pthread_spin_unlock(&curr->state);
	

	return ret;
	
	
}

/**
 * Print a linked list.
 **/
void ll_print(ll_t *ll)
{
	ll_node_t *curr = ll->head;
	printf("LIST [");
	while (curr) {
		if (curr->key == INT_MAX)
			printf(" -> MAX");
		else
			printf(" -> %d", curr->key);
		curr = curr->next;
	}
	printf(" ]\n");
}

