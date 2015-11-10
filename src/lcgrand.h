#ifndef _LCGRAND_H_
#define _LCGRAND_H_

#include <stdlib.h>
#include <math.h>

float lcgrand(int stream);
void lcgrandst(long zset, int stream);
long lcgrandgt(int stream);

double expon(double mean, int stream);
int   random_integer(float prob_distrib[], int stream);
float uniform(float a, float b, int stream);
float erlang(int m, float mean, int stream);

#endif
