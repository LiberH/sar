#define _XOPEN_SOURCE 700

#include "jacobi.h"

#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <mpi.h>

#ifdef _OPENMP
#include <omp.h>
#endif

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

double *
jacobi (double *xA, double *xB, double *xC, double *A, double *b, int h, int w,
	int rank, int size)
{
  int li, lj, i, j, cpt, convergence, iter; 
  int to, from;
  double d, ldelta, delta;
  double *x, *X, /**Y,*/ *t;

  MPI_Status status;

  X = xA;
  /*Y = xB;*/
  x = xC;

  #pragma omp parallel
  {
    #pragma omp for
    for (li = 0; li < h; li++)
      {
	X[li] = 1.0;
	/*Y[li] = 1.0;*/
      }
  }

  iter = 0;
  to   = (rank -1 + size) % size;
  from = (rank +1 + size) % size;

  do
    {
      iter++;
      ldelta = 0.0;

      #pragma omp parallel
      {
        #pragma omp for
        for (li = 0; li < h; li++)
	  x[li] = b[li];
      } /* end omp parallel */

      cpt = rank;
      do
	{
          #pragma omp parallel
          {
            #pragma omp for private(i,j)
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
	  } /* end omp parallel */

	  pthread_mutex_lock (&mutex);
	  MPI_Send (X, h, MPI_DOUBLE, to, 0, MPI_COMM_WORLD);
	  pthread_mutex_unlock (&mutex);
	  /*
	  t = X;
	  X = Y;
	  Y = t;
	  */
	  pthread_mutex_lock (&mutex);
	  MPI_Recv (X, h, MPI_DOUBLE, from, 0, MPI_COMM_WORLD, &status);
	  pthread_mutex_unlock (&mutex);

	  cpt = (cpt + 1) % size;
	}
      while (cpt != rank);
      
      #pragma omp parallel
      {
        #pragma omp for private(i)
        for (li = 0; li < h; li++)
	  {
	    i = li + rank * h;
	    x[li] /= A[li * w + i];
	    
	    #pragma omp critical
	    {
	      d = fabs (X[li] - x[li]);
	      if (d > ldelta) ldelta = d;
	    }
	}
      } /* end omp parallel */

      t = X;
      X = x;
      x = t;

      pthread_mutex_lock (&mutex);
      MPI_Allreduce (&ldelta, &delta, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
      pthread_mutex_unlock (&mutex);

      convergence = (delta < JACOBI_EPS);
    }
  while ((!convergence) && (iter < JACOBI_MAX_ITER));

  /*printf ("iter = %d\n", iter);*/
  return X;
}

int
main (int argc, char **argv)
{
  int mpi_rank, mpi_size, mpi_thread;

  int n, ln;
  double *A, *b, *xA, *xB, *xC, *r, *X, *x;
  double lmax, max;
  struct timeval before, after;

  MPI_Init_thread (&argc, &argv, MPI_THREAD_SERIALIZED, &mpi_thread);
  if (mpi_thread != MPI_THREAD_SERIALIZED)
    {
      fprintf (stderr, "MPI_Init_thread: Thread level provided differs with thread level requierd\n");
      pthread_mutex_lock (&mutex);
      MPI_Finalize ();
      pthread_mutex_unlock (&mutex);
      return EXIT_FAILURE;
    }

  pthread_mutex_lock (&mutex);
  MPI_Comm_size (MPI_COMM_WORLD, &mpi_size);
  MPI_Comm_rank (MPI_COMM_WORLD, &mpi_rank);
  pthread_mutex_unlock (&mutex);
  
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
      pthread_mutex_lock (&mutex);
      MPI_Finalize ();
      pthread_mutex_unlock (&mutex);
      return EXIT_FAILURE;
    }

  b = (double *) calloc (ln, sizeof(double));
  if (b == NULL)
    {
      perror ("calloc");
      pthread_mutex_lock (&mutex);
      MPI_Finalize ();
      pthread_mutex_unlock (&mutex);
      return EXIT_FAILURE;
    }
  
  xA = (double *) calloc (ln, sizeof(double));
  if (xA == NULL)
    {
      perror ("calloc");
      pthread_mutex_lock (&mutex);
      MPI_Finalize ();
      pthread_mutex_unlock (&mutex);
      return EXIT_FAILURE;
    }

  xB = (double *) calloc (ln, sizeof(double));
  if (xB == NULL)
    {
      perror ("calloc");
      pthread_mutex_lock (&mutex);
      MPI_Finalize ();
      pthread_mutex_unlock (&mutex);
      return EXIT_FAILURE;
    }
    
   xC = (double *) calloc (ln, sizeof(double));
   if (xC == NULL)
    {
      perror ("calloc");
      pthread_mutex_lock (&mutex);
      MPI_Finalize ();
      pthread_mutex_unlock (&mutex);
      return EXIT_FAILURE;
    }

  X = (double *) calloc (n, sizeof(double));
  if (X == NULL)
    {
      perror ("calloc");
      pthread_mutex_lock (&mutex);
      MPI_Finalize ();
      pthread_mutex_unlock (&mutex);
      return EXIT_FAILURE;
    }

  r = (double *) calloc (ln, sizeof(double));
  if (r == NULL)
    {
      perror ("calloc");
      pthread_mutex_lock (&mutex);
      MPI_Finalize ();
      pthread_mutex_unlock (&mutex);
      return EXIT_FAILURE;
    }

  generate_matrix (A, ln, n, mpi_rank);
  generate_vector (b, ln);
  
  gettimeofday(&before, NULL);
  x = jacobi (xA, xB, xC, A, b, ln, n, mpi_rank, mpi_size);
  gettimeofday(&after, NULL);

  pthread_mutex_lock (&mutex);
  MPI_Allgather (x, ln, MPI_DOUBLE,
		 X, ln, MPI_DOUBLE, MPI_COMM_WORLD);
  pthread_mutex_unlock (&mutex);

  /* Compute the residual */
  compute_residual (r, A, X, b, ln, n);

  /* Compute the maximum absolute value of the residual */
  lmax = find_max_abs (r, ln);
  pthread_mutex_lock (&mutex);
  MPI_Reduce (&lmax, &max, 1, MPI_DOUBLE, MPI_MAX, ROOT_NODE, MPI_COMM_WORLD);
  pthread_mutex_unlock (&mutex);
  
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
  pthread_mutex_lock (&mutex);
  MPI_Finalize ();
  pthread_mutex_unlock (&mutex);
  return 0;
}
