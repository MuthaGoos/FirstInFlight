#ifndef CONTROLLER_H
#define CONTROLLER_H


#include "queue.h"

int pid(struct Queue * in, struct Queue * out, float kp, float ki, float kd);
int simulate_pid(struct Queue * in, struct Queue * out, float kp, float ki, float kd);

#endif
