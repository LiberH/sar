#define _XOPEN_SOURCE 700

#include "jacobi.h"

#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <mpi.h>

double *
jacobi (double *x, double *X, double *A, double *b, int h, int w,
	int rank, int size)
{ 
  int li, i, j, convergence, iter; 
  double d, ldelta, delta;

  for (li = 0; li < h; li++)
    x[li] = 1.0;

  iter = 0;
  do
    {
      iter++;
      ldelta = 0.0;
      
      MPI_Allgather (x, h, MPI_DOUBLE,
		     X, h, MPI_DOUBLE, MPI_COMM_WORLD);
 
      for (li = 0; li < h; li++)
	{
	  i = li + rank * h;

	  x[li] = b[li];
	  for (j = 0; j < w; j++)
	    if (j != i)
	      x[li] -= A[li * w + j] * X[j];

	  x[li] /= A[li * w + i];
	  d = fabs (X[i] - x[li]);
	  if (d > ldelta) ldelta = d;
	}

      MPI_Allreduce (&ldelta, &delta, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
      convergence = (delta < JACOBI_EPS);
    }
  while ((!convergence) && (iter < JACOBI_MAX_ITER));

  /*printf ("iter = %d\n", iter);*/
  return x;
}

int
main (int argc, char **argv)
{
  int mpi_rank, mpi_size;

  int n, ln;
  double *A, *b, *x, *r, *X;
  double lmax, max;
  struct timeval before, after;

  MPI_Init (&argc, &argv);
  MPI_Comm_size (MPI_COMM_WORLD, &mpi_size);
  MPI_Comm_rank (MPI_COMM_WORLD, &mpi_rank);
  
  /* Get the argument that indicates the problem size */
  n = JACOBI_DEF_SIZE;
  if (argc == 2)
    n = atoi (argv[1]);
  if (mpi_rank == ROOT_NODE)
    fprintf (stdout, "n = %d\n", n);
  ln = n / mpi_size;
  
  /* Initialize the random seed */
  /* srandom((unsigned int) getpid ()); */

  /* Allocate memory */
  A = (double *) calloc (ln * n, sizeof(double));
  if (A == NULL)
    {
      perror ("calloc");
      MPI_Finalize ();
      return EXIT_FAILURE;
    }

  b = (double *) calloc (ln, sizeof(double));
  if (A == NULL)
    {
      perror ("calloc");
      MPI_Finalize ();
      return EXIT_FAILURE;
    }
  
  x = (double *) calloc (ln, sizeof(double));
  if (x == NULL)
    {
      perror ("calloc");
      MPI_Finalize ();
      return EXIT_FAILURE;
    }

  X = (double *) calloc (n, sizeof(double));
  if (X == NULL)
    {
      perror ("calloc");
      MPI_Finalize ();
      return EXIT_FAILURE;
    }

  r = (double *) calloc (ln, sizeof(double));
  if (r == NULL)
    {
      perror ("calloc");
      MPI_Finalize ();
      return EXIT_FAILURE;
    }

  generate_matrix (A, ln, n, mpi_rank);
  generate_vector (b, ln);
  
  gettimeofday (&before, NULL);
  x = jacobi (x, X, A, b, ln, n, mpi_rank, mpi_size);
  gettimeofday (&after, NULL);

  MPI_Allgather (x, ln, MPI_DOUBLE,
		 X, ln, MPI_DOUBLE, MPI_COMM_WORLD);

  /* Compute the residual */
  compute_residual (r, A, X, b, ln, n);

  /* Compute the maximum absolute value of the residual */
  lmax = find_max_abs (r, ln);
  MPI_Reduce (&lmax, &max, 1, MPI_DOUBLE, MPI_MAX, ROOT_NODE, MPI_COMM_WORLD);
  
  if (mpi_rank == ROOT_NODE)
    display_info (A, x, b, r, n, max, &before, &after);

  /* Free the memory */
  free (A);
  free (b);
  free (x);
  free (X);
  free (r);

  /* Return success */
  MPI_Finalize ();
  return 0;
}
