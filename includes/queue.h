#ifndef QUEUE_H
#define QUEUE_H

#define QUEUE_EMPTY 1
#define QUEUE_FULL  2
#define QUEUE_MISS  3

#define MAX_QUEUE_SIZE 1000



struct QueueNode
{
	struct QueueNode * next;
	struct QueueNode * prev;
	void * data;
};

struct Queue
{
	struct QueueNode * head;
	struct QueueNode * tail;
	int size;
	int free_me;
};


struct Queue * new_queue();
void delete_queue();
int pop_queue(struct Queue *, void **);
int push_queue(struct Queue *, void * data);
int empty_queue(struct Queue *);
//int insert_before_node(*, struct QueueNode*);
int pop_tail(struct Queue *, void **);
int unlink_node(struct Queue *, struct QueueNode*);
struct Queue * copy_queue(struct Queue *);


#endif //QUEUE_H
