#include <stdio.h>

#include "common.h"
#include "simulation.h"

int main(int argc, char** argv)
{

	struct SimulationData * coeffs = tune_coefficients(100);
	if(coeffs)
	{
		printf("Kp: %f\nKi: %f\nKd: %f\nError: %f\n",
				coeffs->kp,
				coeffs->ki,
				coeffs->kd,
				coeffs->sum_error);
	}else
		printf("Failure?\n");


	return 0;

}

/*
int main(int argc, char** argv)
{

	int status = SUCCESS;
	struct Queue * pid_in = new_queue();
	struct Queue * pid_out = new_queue();

	float kp, ki, kd;




	status = pid(pid_in, pid_out, kp, ki, kd)

	if (SUCCESS != status)
	{
		printf("PID Failed\n");
		return FAILURE;
	}

	return SUCCESS;

}

*/
