#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>

#include "common.h"
#include "queue.h"
#include "simulation.h"
#include "controller.h"



/* Generate X signal points between 0 and 1 with gradual changes
 * based on scalar
 *
 * Populates queue with the signal data
 *
 * Returns SUCCESS or FAILURE.
 */
int signal_generator(struct Queue * queue, int num_points, float scalar)
{
	int status = SUCCESS;
	float signal = 0.0;
	float noise = 0.0;
	int iter = 0, total = 0;

	printf("Num Pts: %d\n", num_points);
	int total_points = max(min(MAX_SIGNALS, num_points), 0);

	if(total_points == 0)
	{
		printf("Not Generating Any Signals\n");
		return FAILURE;
	}
	printf("Generating %d signals\n", total_points);

	//Seed time for some good random numbers
	srand(time(NULL));

	while (SUCCESS == status && total < total_points) 
	{

		int window_size = (rand() % 100) + 1;
		int sign = (rand() % 10) < 3 ? -1 : 1;
		//printf("New Window Size: %d\n", window_size);

		while(iter < window_size && total < total_points)
		{

			//this will give us a number somwhere between 0 and 1
			noise = scalar*((float)rand()/RAND_MAX);

			//scale the noise by some number to introduce only gradual change
		        signal += sign*noise;

			signal = signal < CLAMP_LOW ? CLAMP_LOW : signal;
			signal = signal > CLAMP_HIGH ? CLAMP_HIGH : signal;

			printf("Random Signal (%d/%d): %f\n", total+1, total_points, signal);

			float* data = (float*) malloc(sizeof(float));
			if(NULL == data)
			{
				printf("Failed to malloc more signal\n");
				return FAILURE;
			}
			*data = signal;
			status = push_queue(queue, (void*)data);
			if(QUEUE_FULL == status)
			{
				printf("Queue Full!\n");
				return SUCCESS;
			}
			if(FAILURE == status)
			{	
				printf("Failed to push to queue\n");
				return FAILURE;
			}
			iter++;
			total++;
		}
		iter = 0;
	}
	//printf("Done Generating Signal!\n");
	return SUCCESS;
}

/* Compare two queues node by node and return the difference summed 
 * between all the nodes
 *
 * Sets &error to be the sum of all errors
 *
 * returns SUCCESS or FAILURE
 */

int find_error_sum(struct Queue * queue1, struct Queue * queue2, float * error)
{
	struct QueueNode * node1;
	struct QueueNode * node2;
	float sum = 0.0;
	float diff = 0.0;
	int item = 0;
	int status = SUCCESS;

	float val1;
	float val2;

	//printf("Finding error sum\n");

	if(NULL == queue1 || NULL == queue2)
		return FAILURE;

	if(queue1->size != queue2->size)
		return FAILURE;

	node1 = queue1->head;
	node2 = queue2->head;

	//printf("Queue1: %p Queue2: %p\n", queue1, queue2);
	//printf("Iterating nodes 1: %p 2: %p\n", node1, node2);

	while(node1 && node2)
	{
		val1 = *(float*) (node1->data);
		val2 = *(float*) (node2->data);

		diff = fabs(val1 - val2);
		//printf("Difference: %f - %f = %f\n", val1,val2, diff);

		sum += diff;
		item++;

		node1 = node1->next;
		node2 = node2->next;
	}

	printf("Error SUM: %f\n", sum);
	*error = sum;
	return SUCCESS;

}

/* Run pid with the desired signals and coefficients, then return the error
 * between what the signal was, and what we expected the PID to be
 *
 * Fills &error with the sum of all errors
 *
 * returns SUCCESS or FAILURE return the error
 */
int simulate(struct Queue * signal_q, float kp, float ki, float kd, float* error)
{
	int status = SUCCESS;
	float sum_error = 0;

	//printf("Simulating\n");
	//Create a new queue that will contain the results of our pid
	struct Queue * pid_q = new_queue();
	if(NULL == pid_q)
	{
		printf("Failed to create a queue for pid\n");
		return FAILURE;
	}

	//Run the pid algorithm
	status = simulate_pid(signal_q, pid_q, kp, ki, kd);
	if(FAILURE == status)
	{
		printf("Pid failed for some reason\n");
		delete_queue(&pid_q);
		return FAILURE;
	}

	//Find the total sum of errors
	status = find_error_sum(signal_q, pid_q, error);
	if( FAILURE == status)
	{
		printf("Failed to find the error sum\n");
		delete_queue(&pid_q);
		return FAILURE;
	}

	
	//Clean up the queues we used
	delete_queue(&pid_q);

	//printf("Simulation Over\n");
	return SUCCESS;
}

/* Fill the coeff_queue with X number of random items
 *
 * Returns SUCCESS or FAILURE
 */
int noisy_coefficients(struct Queue * coeff_q, int num_items)
{
	int status = SUCCESS;
	int iter = 0;

	printf("Generating Noisy Coeffs!\n");
	if( NULL == coeff_q)
		return FAILURE;

	if (num_items <= 0)
		return FAILURE;
	
	while(SUCCESS == status && iter < num_items)
	{

		struct SimulationData * results = (struct SimulationData *)malloc(sizeof(struct SimulationData));

		if(NULL == results)
			return FAILURE;

		results->kp = (float)rand()/(float)RAND_MAX;
		results->ki = (float)rand()/(float)RAND_MAX;
		results->kd = (float)rand()/(float)RAND_MAX;

		results->sum_error = 0.0;
		printf("Noisy Coefficients (%d/%d): kp: %f ki %f kd %f\n",
			       	iter+1,
			       	num_items,
			       	results->kp,
			       	results->ki,
			       	results->kd);

		status = push_queue(coeff_q, results);
		iter++;
	}

	return status;
}

/* Retains the top 10% of coefficients
 * Averages the top 10% a variety of ways
 * Adds slight changes to the top 10%
 * Fills the rest with noisy coefficients
 */
int mutate_coefficients(struct Queue * coeff_q)
{

	struct Queue * top_10p = NULL;
	struct QueueNode * iter_node = NULL;
	struct QueueNode * new_node = NULL;
	struct QueueNode * prev = NULL;
	struct QueueNode * next = NULL;
	struct SimulationData * coeff_data = NULL;
	struct SimulationData * top_data = NULL;
	int size = 0;
	int status = SUCCESS;

	printf("Mutating Coefficients\n");

	if ( NULL == coeff_q)
	{
		printf("Coeff_q is NULL\n");	
		return FAILURE;
	}

	if(coeff_q->size < MAX_COEFFICIENTS)
	{
		printf("Need bigger coeff queue for mutation\n");
		return FAILURE;
	}
	
	top_10p = new_queue();
	if(NULL == top_10p)
	{
		printf("Failed to generate memory to hold top 10p\n");
		return FAILURE;
	}

	size = coeff_q->size/2;
	iter_node = coeff_q->head;

	while(SUCCESS == status)
	{	
		status = pop_queue(coeff_q, (void**)&coeff_data);
		if(SUCCESS != status)
		{
			if(FAILURE == status)
			{
				printf("Unable to pop from coefficient queue, size %d\n", coeff_q->size);		
				goto return_failure;
			}
			break;
		}
		if(NULL == coeff_data)
		{
			printf("Weird, simulated data from coeff_q is NULL\n");
			goto return_failure;
		}

		//If the head exists then we need to scan through looking for an insert point
		//keep the queue sorted with smallest error first
		if(top_10p->head)
		{
			iter_node = top_10p->head;
			//printf("Head Sum Error: %f\n", ((struct SimulationData *)iter_node->data)->sum_error);
			while(iter_node)
			{
				//printf("Iter Node: %p next: %p\n", iter_node, iter_node->next);

				top_data = iter_node->data;
				if(NULL == top_data)
				{
					printf("Weird, top_simu_data is NULL?\n");
					goto return_failure;
				}
				if(top_data->sum_error > coeff_data->sum_error)
				{
					new_node = (struct QueueNode *)malloc(sizeof(struct QueueNode));
					if(NULL == new_node)
					{
						printf("Failed to create a new top10 node\n");
						goto return_failure;
					}
					memset(new_node, 0, sizeof(struct QueueNode));
					new_node->data = coeff_data;

					printf("Inserting midway putting sum: %f before %f\n", coeff_data->sum_error, top_data->sum_error);

					new_node->next = iter_node;
					new_node->prev = iter_node->prev;
					iter_node->prev = new_node;	
					
					//if(iter_node->prev)
					//	iter_node->prev->next = new_node;

					if(NULL == new_node->prev)
					{
						printf("Replacing head %f with %f\n", ((struct SimulationData *)top_10p->head->data)->sum_error,coeff_data->sum_error);
						top_10p->head = new_node;
					}

					top_10p->size++;


					//We've found what we're looking for, no need to continue searching
					break;
					
				}
				//If we haven't reached a large enough size and we're at the end, just go ahead and push
				else if(top_10p->size < size && NULL == iter_node->next)
				{
					printf("Inserting a new node (%d/%d) with sum: %f\n", top_10p->size, size, coeff_data->sum_error);
					status = push_queue(top_10p, coeff_data);
					if(SUCCESS != status)
					{
						printf("Failed to create a new top10 node\n");
						goto return_failure;
					}
					break;
				}
				//printf("Sum_data %f\n", ((struct SimulationData *)iter_node->data)->sum_error);

				iter_node = iter_node->next;
			}	

		} 
		else //if(NULL == top_10p->head)
		{
			status = push_queue(top_10p, coeff_data);
			if(SUCCESS != status)
			{
				printf("Unable to push to top10p queue, size %d\n", top_10p->size);
				goto return_failure;
			}
			//printf("Pushing Initial node to top10\n");

		} 


		while (top_10p->size > size)
		{

			//printf("Pruning Node!\n");
			status = pop_tail(top_10p, NULL);
			if(SUCCESS != status)
			{
				printf("Failed to pop tail off queue, size is %d\n", top_10p->size);
				goto return_failure;

			}
			//printf("Finished pruning\n");
		}
	}	

	status = SUCCESS;
	int i = 0;	
	printf("Top_10p size: %d\n", top_10p->size);
	while(SUCCESS == status)
	{
		status = pop_queue(top_10p, (void**)&coeff_data);
		if(SUCCESS != status)
		{
			if(FAILURE == status)
			{
				printf("Unable to transfer top10_p elements to coefficients 1\n");
				goto return_failure;
			}
			break;
		}
		printf("Selection #%d: kp %f ki %f kd %f error: %f\n", i, coeff_data->kp, coeff_data->ki, coeff_data->kd, coeff_data->sum_error);
		status = push_queue(coeff_q, (void*)coeff_data);
		if(SUCCESS !=  status)
		{
			printf("Unable to transfer top10_p elements to coefficients 2\n");
			goto return_failure;
		}
		i++;
	}

	status = noisy_coefficients(coeff_q, MAX_COEFFICIENTS - coeff_q->size);
	if(FAILURE == status)
	{
		printf("Failed to repopulate coefficients\n");
		goto return_failure;
	}

	delete_queue(&top_10p);
	return SUCCESS;

	
return_failure:
	delete_queue(&top_10p);
	return FAILURE;
}

/* 1 Generate random coefficients and shove them in a queue
 * 2 Generate random signals and shove them into a queue
 * 3 Create an empty error_sum queue
 * 4 Run an iteration of pid using coefficients from above
 * 5 Mutate the coefficients
 * 6 Go to 4 and repeat for X generations
 *
 */
struct SimulationData * tune_coefficients(int generations)
{
	struct QueueNode * node = NULL;
	struct QueueNode * best_node = NULL;
	struct SimulationData * coeffs = NULL;
	struct SimulationData * best_coeffs = NULL;
	int curr_gen = 0;
	int iter = 0;
	int status = FAILURE;

	srand(time(NULL));

	struct Queue * coeff_q = new_queue();
	if ( NULL == coeff_q)
	{
		printf("Failed to create a coeff queue\n");
		return NULL;
	}

	struct Queue * results_q = new_queue();
	if(NULL == results_q)
	{
		printf("Failed to create a new queue for results\n");
		delete_queue(&coeff_q);
		return NULL;
	}

	struct Queue * signal_q = new_queue();
	if(NULL == signal_q)
	{
		printf("Failed to create new signal queue for simulation\n");
		delete_queue(&results_q);
		delete_queue(&coeff_q);
		return NULL;
	}

	status = signal_generator(signal_q, MAX_SIGNALS, 0.01);
	if(FAILURE == status)
	{
		printf("Failed to generate a signal queue\n");
		goto delete_all_queues;
	}

	//Generate X numbers of random kp, kis, and kds
	status = noisy_coefficients(coeff_q, MAX_COEFFICIENTS);
	if(FAILURE == status)
	{
		printf("Failed to generate noisy coefficients\n");
		goto delete_all_queues;
	}

	printf("Beginning Genetic Algorithm\n");
	while(curr_gen < generations)
	{

		iter = 0;
		node = coeff_q->head;
		while(node)
		{

			coeffs = (struct SimulationData *)(node->data);
			printf("Simulating: kp %f ki %f kd %f\n", coeffs->kp, coeffs->ki, coeffs->kd);
			status = simulate(signal_q, 
					coeffs->kp,
				       	coeffs->ki,
				       	coeffs->kd,
				       	&(coeffs->sum_error));

			if(FAILURE == status)
			{
				printf("Simulation failed\n");
				goto delete_all_queues;
			}

			status = push_queue(results_q, coeffs);
			if (SUCCESS != status)
			{
				printf("Failed to push result node");
				goto delete_all_queues;
			}
			iter++;
			node = node->next;
		}

		status = mutate_coefficients(results_q);
		if( SUCCESS != status)
		{
			printf("Mutation failed!\n");
			goto delete_all_queues;
		}
		curr_gen++;
	}

	//Now that all the mutation is done, grab the best node!
	node = results_q->head;
	while(node)
	{
		coeffs = (struct SimulationData *)(node->data);
		if(best_coeffs->sum_error < coeffs->sum_error || !best_coeffs)
		{
		 	best_coeffs = coeffs;
			best_node = node;
		}

		node = node->next;
	}

	status = unlink_node(results_q, best_node);
	if(SUCCESS == status)
		free(best_node);
	

delete_all_queues:

	if(FAILURE == status)
		printf("Failed in Iteration #%d of Gen# %d\n", iter, curr_gen);

	delete_queue(&signal_q);
	delete_queue(&coeff_q);
	delete_queue(&results_q);

	return best_coeffs;
	
}


