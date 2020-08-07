#ifndef SIMULATION_H
#define SIMULATION_H

#include "queue.h"

#define MAX_COEFFICIENTS 300
#define MAX_GENERATIONS 100
#define MAX_SIGNALS 500

#define CLAMP_HIGH 1.0
#define CLAMP_LOW 0.0

struct SimulationData
{
	float kp;
	float ki;
	float kd;

	float sum_error;
};


int signal_generator(struct Queue * out, int num_points, float scalar);
int find_error_sum(struct Queue * q1, struct Queue * q2, float* error);
int simulate(struct Queue * in_q, float kp, float ki, float kd, float* error);
int noise_coefficients(struct Queue * coeff_q, int num_items);

int mutate_coefficients(struct Queue* coeff_queue);
struct SimulationData * tune_coefficients(int generations);


#endif //SIMULATION_H


