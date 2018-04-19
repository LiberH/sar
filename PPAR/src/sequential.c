#define _XOPEN_SOURCE 700

#include "jacobi.h"

#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

double *
jacobi (double *xA, double *xB, double *A, double *b, int n)
{ 
  int i, j, convergence, iter; 
  double d, delta;
  double *x, *X, *t;

  X = xA;
  x = xB;
  for (i = 0; i < n; i++)
    X[i] = 1.0;

  iter = 0;
  do
    {
      iter++;
      delta = 0.0;

      for (i = 0; i < n; i++)
	{
	  x[i] = b[i];
	  for (j = 0; j < n; j++)
	    if (j != i)
	      x[i] -= A[i * n + j] * X[j];
	   
	  x[i] /= A[i * n + i];
	  d = fabs (X[i] - x[i]);
	  if (d > delta) delta = d;
	}

      t = X;
      X = x; 
      x = t;
      convergence = (delta < JACOBI_EPS);
    }
  while ((!convergence) && (iter < JACOBI_MAX_ITER));
  
  /*printf ("iter = %d\n", iter);*/
  return X;
}

/* A small testing main program */
int main(int argc, char *argv[]) {
  int n;
  double *A, *b, *x, *r, *xA, *xB;
  double maxAbsRes;
  struct timeval before, after;

  /* Get the argument that indicates the problem size */
  n = JACOBI_DEF_SIZE;
  if (argc == 2)
    n = atoi (argv[1]);
  fprintf (stdout, "n = %d\n", n);

  /* Initialize the random seed */
  /* srandom((unsigned int) getpid()); */

  /* Allocate memory */
  if ((A = (double *) calloc(n * n, sizeof(double))) == NULL) {
    fprintf(stderr, "Not enough memory.\n");
    return 1;
  }

  if ((b = (double *) calloc(n, sizeof(double))) == NULL) {
    free(A);
    fprintf(stderr, "Not enough memory.\n");
    return 1;
  }
  
  if ((xA = (double *) calloc(n, sizeof(double))) == NULL) {
    free(A);
    free(b);
    fprintf(stderr, "Not enough memory.\n");
    return 1;
  }

  if ((xB = (double *) calloc(n, sizeof(double))) == NULL) {
    free(A);
    free(b);
    free(xA);
    fprintf(stderr, "Not enough memory.\n");
    return 1;
  }

  if ((r = (double *) calloc(n, sizeof(double))) == NULL) {
    free(A);
    free(b);
    free(xA);
    free(xB);
    fprintf(stderr, "Not enough memory.\n");
    return 1;
  }

  /* Generate a random diagonally dominant matrix A and a random
     right-hand side b 
  */
  generate_matrix (A, n, n, 0);
  generate_vector(b, n);
  
  /* Perform Jacobi iteration 

     Time this (interesting) part of the code.

  */
  gettimeofday(&before, NULL);
  x = jacobi (xA, xB, A, b, n);
  gettimeofday(&after, NULL);

  /* Compute the residual */
  compute_residual(r, A, x, b, n, n);

  /* Compute the maximum absolute value of the residual */
  maxAbsRes = find_max_abs (r, n);
  
  display_info (A, x, b, r, n, maxAbsRes, &before, &after);

  /* Free the memory */
  free(A);
  free(b);
  free(xA);
  free(xB);
  free(r);

  /* Return success */
  return 0;
}
