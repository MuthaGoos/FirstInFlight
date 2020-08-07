#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "common.h"
#include "queue.h"


#define MAX_ITERATIONS 10000
#define USEC_CONSTANT 1000000

int pid(struct Queue * read_q, struct Queue * write_q, float kp, float ki, float kd)
{
/* Information about implementing a PID from a variety of sources:
 *
 * http://robotsforroboticists.com/pid-control/
 */
	
	/* Proportional term for controlling error.
	 * If this value is proportional to error correction. 
	 * Higher values means greater changes, smaller values mean smaller changes
	 */
	//float kp = 1.0;

	/* Integral term for controlling errors that accumulate.
	 * A large ki tries to correct overtime and may overshoot values.
	 * This is the term to investigate most closely.
	 */
	//float ki = 1.0;

	/* Derivative term looks at how the system is behaving between measurement
	 * intervals.  This helps dampen the system to improve stability.  Many
	 * motor controlls will only configure PI controllers, and no derivative
	 */
	//float kd = 1.0;

	float bias = 0;
	float error = 0;
 	float error_prior = 0;
	float desired = 0; 
	float * measured;
	float integral = 0;
	float integral_prior = 0;
	float derivative; 

	float delta_t = 0.2;
	float * out = NULL;

	int status = SUCCESS;
	int i = 0;

	if ( NULL == read_q || NULL == write_q)
		return FAILURE;

	struct QueueNode* iter_node = read_q->head;

	while (iter_node)
	{

		//printf("Calculating PID output: %d remain\n", read_q->size);
		//iter_node = 
		//status = pop_queue(read_q, (void*)&measured);
		//if(SUCCESS != status)
		//	break;



		//if(FAILURE == status)

		error = desired - *measured;

		//free(measured);

		//scale the error across 
		integral = integral_prior + error * delta_t;

		derivative = (error - error_prior)/delta_t;

		out = (float *)malloc(sizeof(float));
		if(NULL == out)
		{
			printf("Failed top malloc size for float\n");
			return FAILURE;
		}


		*out = kp*error + ki*integral + kd*derivative + bias;

		//printf("pushing %p -> %f\n", out, *out);
		status = push_queue(write_q, (void*)out);
		if(FAILURE == status)
		{
			printf("Exit status: %d :  %d iterations\n", status, i);
			break;
		}

		error_prior = error;
		
		integral_prior = integral;
		iter_node = iter_node->next; 
		//usleep(delta_t * USEC_CONSTANT);
		i++;

	}
	if(FAILURE != status)
		status = SUCCESS;

	printf("Returning from PID loop: Status %d\n", status);
	return status;
}

