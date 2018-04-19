#define _XOPEN_SOURCE 700

#include "jacobi.h"

#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <mpi.h>

double *
jacobi (double *xA, double *xB, double *xC, double *A, double *b, int h, int w,
	int rank, int size)
{
  int li, lj, i, j, cpt, convergence, iter; 
  int to, from;
  double d, ldelta, delta;
  double *x, *X, *t, *buf;

  MPI_Status status;

  X = xA;
  x = xB;
  buf = xC;
  for (li = 0; li < h; li++)
    X[li] = 1.0;

  iter = 0;
  to   = (rank -1 + size) % size;
  from = (rank +1 + size) % size;

  do
    {
      iter++;
      ldelta = 0.0;

      for (li = 0; li < h; li++)
	x[li] = b[li];
      
      cpt = rank;
      do
	{
	  for (li = 0; li < h; li++)
	    {
	      i = li + rank * h;
	      for (lj = 0; lj < h; lj++)
		{
		  j = lj + cpt * h;
		  if (j != i)
		    x[li] -= A[li * w + j] * X[lj];
		}
	    }

	  if (size > 1)
	    {
	      if (rank % 2)
		{
		  MPI_Send (X, h, MPI_DOUBLE, to,   0, MPI_COMM_WORLD);  
		  MPI_Recv (X, h, MPI_DOUBLE, from, 0, MPI_COMM_WORLD, &status);
		}
	      else
		{
		  MPI_Recv (buf, h, MPI_DOUBLE, from, 0, MPI_COMM_WORLD, &status);
		  MPI_Send (X, h, MPI_DOUBLE, to, 0, MPI_COMM_WORLD);
		  t = X;
		  X = buf;
		  buf = t;
		}
	    }

	  cpt = (cpt + 1) % size;
	}
      while (cpt != rank);
      
      for (li = 0; li < h; li++)
	{
          i = li + rank * h;
	  x[li] /= A[li * w + i];

	  d = fabs (X[li] - x[li]);
	  if (d > ldelta) ldelta = d;
	}

      t = X;
      X = x;
      x = t;
      MPI_Allreduce (&ldelta, &delta, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
      convergence = (delta < JACOBI_EPS);
    }
  while ((!convergence) && (iter < JACOBI_MAX_ITER));

  /*printf ("iter = %d\n", iter);*/
  return X;
}

int
main (int argc, char **argv)
{
  int mpi_rank, mpi_size;

  int n, ln;
  double *A, *b, *xA, *xB, *xC, *r, *X, *x;
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
  /*srandom((unsigned int) getpid ());*/

  /* Allocate memory */
  A = (double *) calloc (ln * n, sizeof(double));
  if (A == NULL)
    {
      perror ("calloc");
      MPI_Finalize ();
      return EXIT_FAILURE;
    }

  b = (double *) calloc (ln, sizeof(double));
  if (b == NULL)
    {
      perror ("calloc");
      MPI_Finalize ();
      return EXIT_FAILURE;
    }
  
  xA = (double *) calloc (ln, sizeof(double));
  if (xA == NULL)
    {
      perror ("calloc");
      MPI_Finalize ();
      return EXIT_FAILURE;
    }

  xB = (double *) calloc (ln, sizeof(double));
  if (xB == NULL)
    {
      perror ("calloc");
      MPI_Finalize ();
      return EXIT_FAILURE;
    }

  xC = (double *) calloc (ln, sizeof(double));
  if (xC == NULL)
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
  
  gettimeofday(&before, NULL);
  x = jacobi (xA, xB, xC, A, b, ln, n, mpi_rank, mpi_size);
  gettimeofday(&after, NULL);

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
  free (xA);
  free (xB);
  free (xC);
  free (X);
  free (r);

  /* Return success */
  MPI_Finalize ();
  return 0;
}
