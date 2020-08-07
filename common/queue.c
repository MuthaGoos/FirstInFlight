#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "queue.h"

struct Queue * new_queue()
{
	struct Queue * queue;
	
	queue = (struct Queue *) malloc(sizeof(struct Queue));
	if(NULL == queue)
	{	
		return queue;
	}

	memset(queue, 0, sizeof(struct Queue));
	return queue;
}

void delete_queue(struct Queue ** queue)
{
	empty_queue(*queue);
	free(*queue);
	*queue = NULL;
}

int pop_queue(struct Queue * queue, void** data)
{
	struct QueueNode * node;

	if (NULL == queue)
		return FAILURE;

	if (0 == queue->size)
		return QUEUE_EMPTY;

	if (NULL == queue->head)
		return FAILURE;

	if (NULL == data)
		return FAILURE;

	//Save the node we popped so we can free it later
	node = queue->head;

	//printf("Popping: %p (%p) into %p\n", node, node->data, data);
	//Populate the user's data with that saved in the queue
	*data = node->data;

	//Overwrite the head node with the next one
	queue->head = node->next;

	if(NULL == queue->head)
		queue->tail = NULL;
	else
		queue->head->prev = NULL;

	//Delete the old node
	free(node);

	queue->size--;
	return SUCCESS;
}

int push_queue(struct Queue * queue, void * data)
{
	struct QueueNode * node;

	//printf("Pushing to queue: %p\n", data);
	if (NULL == queue)
	{
		printf("queue is NULL\n");
		return FAILURE;
	}

	if (MAX_QUEUE_SIZE == queue->size)
	{
		printf("Queue is FULL\n");
		return QUEUE_FULL;
	}

	//Create a new node structure
	node = (struct QueueNode *) malloc(sizeof(struct QueueNode));
	if (NULL == node)
	{
		printf("Failed to create a new QueueNode\n");
		return FAILURE;
	}

	memset(node, 0, sizeof(struct QueueNode));

	//assign user data to the node
	node->data = data;

	//replace the tail node whether or not it exists
	if (queue->tail)
	{	
		node->prev = queue->tail;	
		queue->tail->next = node;
	}

	if (NULL == queue->head)
		queue->head = node;

	queue->tail = node;
	queue->size++;

	//printf("Finished - %d elements\n", queue->size);
	return SUCCESS;
}

int empty_queue(struct Queue* queue)
{
	void* data;
	int status = SUCCESS;

	while(SUCCESS == status)
	{
		status = pop_queue(queue, &data);
		if(SUCCESS == status)
		{
			free(data);
		}
	}

	if (QUEUE_EMPTY == status)
		return SUCCESS;
	
	return FAILURE;
}

/* If the data is a pointer to something then this is a SHALLOW copy */
struct Queue * copy_queue(struct Queue * queue)
{

	int status = SUCCESS;
	struct Queue * copy_q;
	struct QueueNode * node;

	if (NULL == queue)
		return NULL;

	copy_q = new_queue();
	if (NULL == copy_q)
		return NULL;

	node = queue->head;
	while(node)
	{
		status = push_queue(copy_q, node->data);
		if (SUCCESS != status)
		{
			delete_queue(&copy_q);
			return NULL;
		}
		node = node->next;
	}

	return copy_q;
}

/*
int insert_before_node(struct Queue * queue, struct QueueNode * insert, struct QueueNode * before_me)
{

	if(NULL == insert)
		return FAILURE;
	if(NULL == before_me)
		return FAILURE;




}
*/
int pop_tail(struct Queue * queue, void** data)
{
	struct QueueNode * tail;

	if (NULL == queue)
	{
		printf("Can't pop tail of a NULL queue\n");
		return FAILURE;
	}
	if (0 == queue->size)
		return QUEUE_EMPTY;

	tail = queue->tail;

	//printf("Pruning Tail: %p\n", tail);
	queue->tail = tail->prev;
	queue->tail->next = NULL;

	if (NULL == data)
	{
		if(tail)
		{
			//printf("Tail Data %p\n", tail->data);
			free(tail->data);
		}
	}
	else
		*data = tail->data;

	free(tail);
	queue->size--;
	return SUCCESS;
}


int unlink_node(struct Queue * queue, struct QueueNode* target)
{

	struct QueueNode * iter_node = NULL;
	struct QueueNode * prev;
	struct QueueNode * next;
	int found = 0;

	if(NULL == queue)
	{
		printf("Can't cut node from NULL queue\n");
		return FAILURE;
	}
	if (NULL == target)
	{
		printf("Can't cut empty node?\n");
		return FAILURE;
	}

	iter_node = queue->head;
	while(iter_node)
	{
		if(iter_node == target)
		{
			found = 1;
			break;
		}
		iter_node = iter_node->next;
	}
	
	if(!found)
		return QUEUE_MISS;

	prev = iter_node->prev;
	next = iter_node->next;

	if(prev)
		prev->next = next;
	if(next)
		next->prev = prev;
	return SUCCESS;
}

