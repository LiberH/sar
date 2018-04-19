#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#define FALSE 0
#define TRUE  !FALSE
#define DEFAULT_N 1000000000

struct data {
  char *list;
  int *primes;
  int count;
  long double time;
};

int GPU_process (struct data *gpu, int n);
int CPU_process (struct data *cpu, int n);

int
main (int argc, char **argv)
{
  int n, i, failed;
  struct data gpu, cpu;

  n = DEFAULT_N;
  if (argc > 1)
    n = atoi (argv[1]);

  /* GPU: */
  gpu.count = GPU_process (&gpu, n);
  printf ("GPU: %d primes to %d\n", gpu.count, n);
  printf ("CPU: %Lf seconds\n", gpu.time);

  /* CPU: */
  cpu.count = CPU_process (&cpu, n);
  printf ("CPU: %d primes to %d\n", cpu.count, n);
  printf ("CPU: %Lf seconds\n", cpu.time);

  /* Check: */
  failed = FALSE;
  for (i = 0; i < n; ++i)
    if (gpu.primes[i] != cpu.primes[i])
      {
	failed = TRUE;
	printf ("      %d <> %d (%d)\n", gpu.primes[i], cpu.primes[i], (i +1));
      }

  if (failed)
    printf (" ! : Results differ.\n");

  return EXIT_SUCCESS;
}

void
CPU_init (int **primes, char **list, int n)
{
  *list = (char *) malloc (sizeof (char) * (n +1));
  if (*list == NULL)
    {
      perror ("malloc");
      exit (EXIT_FAILURE);
    }

  *primes = (int *) malloc (sizeof (int) * (n +1));
  if (*primes == NULL)
    {
      perror ("malloc");
      exit (EXIT_FAILURE);
    }
}

void
CPU_sieve (char *list, int n)
{
  int s, k, i;

  s = (int) sqrt (n);
  for (k = 2; k < (s +1); ++k)

    /* Check if not marked: */
    if (!*(list + k))

      /* Mark each multiple: */
      for (i = k * k; i < (n +1); i += k)
	*(list + i) = TRUE;
}

int
CPU_store (int *primes, char *list, int n)
{
  int p, k;

  p = 0;
  for (k = 2; k < (n +1); ++k)

    /* Check if not marked: */
    if (!*(list + k))
      *(primes + p++) = k;

  return p++;
}

int
CPU_process (struct data *cpu, int n)
{
  int p, err;
  struct timeval a, b;

  CPU_init (&cpu -> primes, &cpu -> list, n);

  err = gettimeofday (&a, NULL);
  if (err)
    {
      perror ("gettimeofday");
      exit (EXIT_FAILURE);
    }

  CPU_sieve (cpu -> list, n);
  p = CPU_store (cpu -> primes, cpu -> list, n);

  err = gettimeofday (&b, NULL);
  if (err)
    {
      perror ("gettimeofday");
      exit (EXIT_FAILURE);
    }

  cpu -> time = (b.tv_sec + (b.tv_usec * 1.0e-6L)) -
    (a.tv_sec + (a.tv_usec * 1.0e-6L));
  return p;
}

void
GPU_init (char **cuda_list, int n)
{
  cudaError_t cuda_err;

  cuda_err = cudaMalloc (cuda_list, sizeof (char) * (n +1));
  if (cuda_err != cudaSuccess)
    {
      fprintf (stderr, "cudaMalloc: %s\n", cudaGetErrorString (cuda_err));
      exit (EXIT_FAILURE);
    }

  cuda_err = cudaMemset (*cuda_list, 0, sizeof (char) * (n +1));
  if (cuda_err != cudaSuccess)
    {
      fprintf (stderr, "cudaMemset: %s\n", cudaGetErrorString (cuda_err));
      exit (EXIT_FAILURE);
    }

}

__global__ void
kkk (char *list, int n)
{
  int k;

  k = blockDim.x * blockIdx.x + threadIdx.x;
  printf ("%d. %d\n", k, *(list + k));
  *(list + k) = k;
}

__global__ void
kernel (char *list, int n, int s)
{
  int k, i;

  k = blockDim.x * blockIdx.x + threadIdx.x;
  if (k > 1 && k < (s +1))

    /* Check if not marked: */
    if (!*(list + k))

      /* Mark each multiple: */
      for (i = k * k; i < (n +1); i += k)
	*(list + i) = TRUE;
}

void
GPU_sieve (char *list, char *cuda_list, int n)
{
  int s;
  cudaError_t cuda_err;
  
  s = (int) sqrt (n);
  kernel <<<(s/32 +1), (32)>>> (cuda_list, n, s);
  cuda_err = cudaDeviceSynchronize ();
  if (cuda_err != cudaSuccess)
    {
      fprintf (stderr, "cudaDeviceSynchronize: %s\n", cudaGetErrorString (cuda_err));
      exit (EXIT_FAILURE);
    }

  cuda_err = cudaMemcpy (list, cuda_list, sizeof (char) * (n +1), cudaMemcpyDeviceToHost);
  if (cuda_err != cudaSuccess)
    {
      fprintf (stderr, "cudaMemcpy: %s\n", cudaGetErrorString (cuda_err));
      exit (EXIT_FAILURE);
    }
}

int
GPU_process (struct data *gpu, int n)
{
  int p, err;
  char *cuda_list = NULL;
  struct timeval a, b;

  CPU_init (&gpu -> primes, &gpu -> list, n);
  GPU_init (&cuda_list, n);

  err = gettimeofday (&a, NULL);
  if (err)
    {
      perror ("gettimeofday");
      exit (EXIT_FAILURE);
    }

  GPU_sieve (gpu -> list, cuda_list, n);
  p = CPU_store (gpu -> primes, gpu -> list, n);

  err = gettimeofday (&b, NULL);
  if (err)
    {
      perror ("gettimeofday");
      exit (EXIT_FAILURE);
    }

  gpu -> time = (b.tv_sec + (b.tv_usec * 1.0e-6L)) -
    (a.tv_sec + (a.tv_usec * 1.0e-6L));
  return p;
}
