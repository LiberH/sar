#ifndef _JACOBI_H_
#define _JACOBI_H_

#include <sys/time.h>

#define JACOBI_EPS      1e-13
#define JACOBI_DEF_SIZE 66000
#define JACOBI_MAX_ITER 100
#define JACOBI_MAX_RESI 1e-7
#define JACOBI_DIS_SIZE 4

#define ROOT_NODE 0

double time_diff (struct timeval *, struct timeval *);
double random_ouble (double);
void generate_matrix (double *, int, int, int);
void generate_vector (double *, int);
void compute_residual (double *, double *, double *, double *, int, int);
double find_max_abs (double *, int);
void display_info (double *, double *, double *, double *, int,
		   double, struct timeval *, struct timeval *);
#endif /* _JACOBI_H_ */
