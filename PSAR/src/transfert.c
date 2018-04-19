#define _XOPEN_SOURCE 700

#include "losh.h"
#include "master.h"
#include "slave.h"
#include "information.h"
#include "localisation.h"
#include "transfert.h"
#include "utils.h"
#include "xdr.h"

#include <sys/wait.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void *execute_routine (void *);

/* Local */

void
execute (char **args)
{
  int err;
  pthread_t tid;
  struct task task, *temp;
  struct slave peer;

  task_init (&task, args);
  temp = (struct task *) list_add (&slave -> tasks,
				   (void *) &task,
				   (cpyfct_t) task_copy);
  slave -> task_number++;

  switch (losh_type)
    {
    case SHARING:
      /* If my load is inferior to the threshold I'll ask a peer: */
      slave -> load = self_load ();
      if (slave -> load < THRESHOLD)
	break;

    case BALANCING:
      /* If I'm the peer returned by the master I'll execute locally: */
      locateslave_call (&peer);
      if (!strcmp (peer.addr, slave -> addr))
	break;

      /* Remote execution: */
      recordtask_call (&peer, &task);
      strcpy (temp -> addr, peer.addr);
      return;
    }

  /* Local execution: */
#ifdef DEBUG
  printf ("SLAV (info) local execution\n");
  task_print (temp);
#endif
  err = pthread_create (&tid, NULL, execute_routine, (void *) temp);
  if (err)
    {
      fprintf (stderr, "pthread_create: Failed.\n");
      exit (EXIT_FAILURE);
    }
}

/* RECORDTASK */

void
recordtask_call (struct slave *peer, struct task *task)
{
  int err;
  CLIENT *clnt;
  struct timeval timeout;
  
#ifdef DEBUG
  printf ("SLAV [call] recordtask\n");
#endif
  clnt = clnt_create (peer -> addr, LOSH, SLAVE, "udp");
  if (clnt == NULL)
    {
      fprintf (stderr, "clnt_create: Failed.\n");
      exit (EXIT_FAILURE);
    }

  timeout.tv_sec  = 10;
  timeout.tv_usec =  0;
  err = clnt_call (clnt, RECORDTASK,
                   (xdrproc_t) xdr_task, (char *) task,
                   (xdrproc_t) xdr_void, (char *) NULL,
                   timeout);
  if (err)
    {
      fprintf (stderr, "clnt_call: Failed.\n");
      exit (EXIT_FAILURE);
    }

  clnt_destroy (clnt);
#ifdef DEBUG
  printf ("SLAV [resp] recordtask\n");
#endif
}

void *
recordtask (void *arg, struct svc_req *request)
{
  int err;
  pthread_t tid;
  static caddr_t reply;
  struct task *task;

  task = (struct task *) arg;

  task = (struct task *) list_add (&slave -> imported_tasks,
				   (void *) task, (cpyfct_t) task_copy);
  slave -> imported_task_number++;
#ifdef DEBUG
  printf ("SLAV [exec] recordtask\n");
  task_print (task);
  slave_print (slave);
#endif

  /* Create thread: */
  err = pthread_create (&tid, NULL, execute_routine, (void *) task);
  if (err)
    {
      fprintf (stderr, "pthread_create: Failed.\n");
      exit (EXIT_FAILURE);
    }

  reply = NULL;
  return (void *) &reply;
}

void *
execute_routine (void *arg)
{
  int err;
  struct task *task;
  struct slave peer;

  task = (struct task *) arg;
  err = pthread_detach (pthread_self ());
  if (err)
    {
      perror ("pthread_detach");
      exit (EXIT_FAILURE);
    }

  task -> pid = fork ();
  if (task -> pid == -1)
    {
      perror ("fork");
      exit (EXIT_FAILURE);
    }

  if (!task -> pid)
    {
      execv (task -> path, task -> argv);
      perror ("execvp");
      exit (EXIT_FAILURE);
    }

  task -> pid = wait (&task -> status);
  if (task -> pid == -1)
    {
      perror ("waitpid");
      exit (EXIT_FAILURE);
    }

  /* Local execution: */
  if (!strcmp (task -> addr, slave -> addr))
    {
      list_del (&slave -> tasks, (void *) task, (cmpfct_t) task_compare);
      slave -> task_number--;
#ifdef DEBUG
      printf ("SLAV (info) ended local execution\n");
      task_print (task);
#endif

      pthread_exit (NULL);
    }

  /* Remote execution: */
  strcpy (peer.addr, task -> addr);
  strcpy (task -> addr, slave -> addr);

  /*if (WIFEXITED (task -> status))*/
    returntask_call (&peer, task);

  pthread_exit (NULL);
}

/* RETURNTASK */

void
returntask_call (struct slave *peer, struct task *task)
{
  int err;
  CLIENT *clnt;
  struct timeval timeout;

#ifdef DEBUG
  printf ("SLAV [call] returntask\n");
#endif
  list_del (&slave -> imported_tasks, (void *) task, (cmpfct_t) task_compare);
  slave -> imported_task_number--;

  clnt = clnt_create (peer -> addr, LOSH, SLAVE, "udp");
  if (clnt == NULL)
    {
      fprintf (stderr, "returntask_call: Peer not found.\n");
      return;
    }

  timeout.tv_sec  = 10;
  timeout.tv_usec =  0;
  err = clnt_call (clnt, RETURNTASK,
                   (xdrproc_t) xdr_task, (char *) task,
                   (xdrproc_t) xdr_void, (char *) NULL,
                   timeout);
  if (err)
    {
      fprintf (stderr, "clnt_call: Failed.\n");
      exit (EXIT_FAILURE);
    }

  clnt_destroy (clnt);
#ifdef DEBUG
  printf ("SLAV [resp] returntask\n");
#endif
}

void *
returntask (void *arg, struct svc_req *request)
{
  static caddr_t reply;
  struct task *task;

  task = (struct task *) arg;

#ifdef DEBUG
  printf ("SLAV [exec] returntask\n");
  slave_print (slave);
#endif
  list_del (&slave -> tasks, (void *) task, (cmpfct_t) task_compare);
  slave -> task_number--;

  reply = NULL;
  return (void *) &reply;
}
