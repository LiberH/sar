#define _XOPEN_SOURCE 700

#include "jacobi.h"

#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

/* Compute time difference in seconds */
double
time_diff (struct timeval *before, struct timeval *after) 
{
  double secs, usecs;
  
  secs  = ((double) after->tv_sec)  - ((double) before->tv_sec);
  usecs = ((double) after->tv_usec) - ((double) before->tv_usec);
  return secs + (usecs / 1000000.0);
}

/* Generate a random double */
double
random_double (double max) {
  double res;

  res = max * ((double) random ()) / ((double) RAND_MAX);
  return (random () % 2) ? -res : res;
}

/* Generate a diagonally dominant matrix */
void
generate_matrix (double *A, int h, int w, int rank)
{
  int i, j, k;

  double max;
  max = 0.5 / ((double) w);
  for (i = 0; i < h; i++)
    {
      for (j = 0; j < w; j++)
	A[i*w +j] = random_double (max);

      k = i + rank * h;
      A[i*w +k] = 2.0 + random_double (1.0);
    }
}

/* Generate a vector */
void
generate_vector (double *v, int n) 
{
  int i;

  for (i = 0; i < n; i++)
    v[i] = random_double (1024.0);
}

/* Compute the residual r = b - A * x */
void
compute_residual (double *r, double *A, double *x, double *b, int h, int w)
{
  int i, j;
  double c;

  for (i = 0; i < h; i++)
    {
      c = b[i];
      for (j = 0; j < w; j++)
	c -= A[i*w +j] * x[j];

      r[i] = c;
    }
}

/* Find max absolute value */
double
find_max_abs (double *v, int n)
{
  int i;
  double c, res;

  res = 0.0;
  for (i = 0; i < n; i++)
    {
      c = fabs (v[i]);
      res = (c > res) ? c : res;
    }

  return res;
}

void
display_info (double *A, double *x, double *b, double *r, int w,
	      double max, struct timeval *before, struct timeval *after)
{
  int i, j;
  /* Display time for the Jacobi iteration */
  printf ("Took %12.6fs\n", time_diff (before, after));

  printf ("A");
  for (i = 0; i < JACOBI_DIS_SIZE; i++)
    {
      for (j = 0; j < JACOBI_DIS_SIZE; j++)
	printf("\t%1.8e", A[i*w+j]);
      printf ("\n");
    }
  printf("\n");

  printf ("b");
  for (i = 0; i < JACOBI_DIS_SIZE; i++)
    printf("\t%1.8e", b[i]);
  printf("\n");

  printf ("x");
  for (i = 0; i < JACOBI_DIS_SIZE; i++)
    printf("\t%1.8e", x[i]);
  printf("\n");

  printf ("r");
  for (i = 0; i < JACOBI_DIS_SIZE; i++)
    printf("\t%1.8e", r[i]);
  printf("\n\n");

  /* Decide if residual is small enough */
  printf("Residual: %1.8e\n", max);
  if (max <= JACOBI_MAX_RESI)
    printf("Solution okay\n");
  else
    printf("Solution NOT okay\n");
}
