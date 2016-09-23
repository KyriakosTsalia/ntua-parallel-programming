#include <stdio.h>
#include <stdlib.h> /* rand() */
#include <limits.h>

#include "../common/alloc.h"
#include "ll.h"

typedef struct ll_node {
	int key;
	struct ll_node *next;
	/* other fields here? */
	int marked;
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
	ret->marked=0;
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

int find(ll_t *ll, int key, ll_node_t **pred, ll_node_t **curr) {

	ll_node_t *succ = NULL;
	*pred = NULL;
	*curr = NULL;
	int snip;
	RETRY: while(1) {
		*pred = ll->head;
		*curr = (*pred)->next;
		while(1) {
			succ = (*curr)->next;
			while((*curr)->marked) {
				snip = __sync_bool_compare_and_swap(&((*pred)->next), *curr, succ);
				if (!snip) goto RETRY;
				*curr = succ;
				succ = (*curr)->next;
			}
			if ((*curr)->key >= key)
				return 1;
			*pred = *curr;
			*curr = succ;
		}
	}
	
}

int ll_remove(ll_t *ll, int key)
{
	int snip;
	ll_node_t *pred;
	ll_node_t *curr;
	while(1) {
		find(ll, key, &pred, &curr);
		if (curr->key != key) {
			return 0;
		}
		else {
			ll_node_t *succ = curr->next;
			snip = __sync_bool_compare_and_swap(&(curr->next), succ, curr->next);
			if (!snip) continue;
			curr->marked = 1;
			__sync_bool_compare_and_swap(&(pred->next), curr, succ);
			return 1;
		}
		
	}
}

int ll_add(ll_t *ll, int key)
{
	int snip;
        ll_node_t *pred;
        ll_node_t *curr;
        while(1) {
                find(ll, key, &pred, &curr);
                if (curr->key == key) {
                        return 0;
                }
                else {
			ll_node_t * node = ll_node_new(key);
			__sync_bool_compare_and_swap(&(node->next), node->next, curr);
			if (__sync_bool_compare_and_swap(&(pred->next), curr, node)) 
				return 1;
                }

        }

}

int ll_contains(ll_t *ll, int key)
{
	ll_node_t *curr = ll->head;
	while (curr->key < key)
		curr = curr->next;
	ll_node_t *succ = curr->next;
	return (curr->key == key) && !(curr->marked);
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
