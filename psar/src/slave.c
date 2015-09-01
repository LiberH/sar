#define _XOPEN_SOURCE 700

#include "losh.h"
#include "master.h"
#include "slave.h"
#include "information.h"
#include "localisation.h"
#include "transfert.h"
#include "ihm.h"
#include "utils.h"
#include "xdr.h"

#include <rpc/rpc.h>
#include <rpc/pmap_clnt.h>

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static struct slave self;
struct slave *slave = &self;
static void *slave_routine (void *);

void
slave_role (void)
{
  int err;
  pthread_t tid;

  /* Init'ing the slave: */
  self_addr (self.addr);
  self.running = TRUE;
  self.load = -1;
  self.tasks = NULL;
  self.task_number = 0;
  printf ("SLAVE  %s\n", self.addr);

  /* Join LoSh as slave: */
  join (SLAVE);

  /* Create slave thread: */
  err = pthread_create (&tid, NULL, slave_routine, NULL);
  if (err)
    {
      fprintf (stderr, "pthread_create: Failed.\n");
      exit (EXIT_FAILURE);
    }

  /* Run slave RCP server: */
  svc_run ();
  fprintf (stderr, "svc_run: Failed.\n");
  exit (EXIT_SUCCESS);
}

void *
slave_routine (void *arg)
{
  int err;

  err = pthread_detach (pthread_self ());
  if (err)
    {
      perror ("pthread_detach");
      exit (EXIT_FAILURE);
    }

  entrerCommande ();
  exit (EXIT_SUCCESS);
}

void
slave_dispatch (struct svc_req *request, SVCXPRT *xprt)
{
  int err;
  union {
    struct task task;
    struct slave slave;
  } in;
  caddr_t out;
  xdrproc_t xdr_inproc, xdr_outproc;
  void *(*function)(void *, struct svc_req *);

  switch (request -> rq_proc)
    {
    case NULLPROC:
      svc_sendreply (xprt, (xdrproc_t) xdr_void, (char *) NULL);
      return;

    case WHOISSLAVE:
      xdr_inproc  = (xdrproc_t) xdr_void;
      xdr_outproc = (xdrproc_t) xdr_slave;
      function = whoisslave;
      break;

    case RECORDTASK:
      xdr_inproc  = (xdrproc_t) xdr_task;
      xdr_outproc = (xdrproc_t) xdr_void;
      function = recordtask;
      break;

    case RETURNTASK:
      xdr_inproc  = (xdrproc_t) xdr_task;
      xdr_outproc = (xdrproc_t) xdr_void;
      function = returntask;
      break;

    case DELETETASK:
      xdr_inproc  = (xdrproc_t) xdr_slave;
      xdr_outproc = (xdrproc_t) xdr_void;
      function = deletetask;
      break;

    default:
      svcerr_noproc (xprt);
      return;
    }

  memset ((void *) &in, 0, sizeof (in));
  err = svc_getargs (xprt, (xdrproc_t) xdr_inproc, (caddr_t) &in);
  if (!err)
    {
      svcerr_decode (xprt);
      return;
    }

  out = (*function) ((void *) &in, request);
  err = svc_sendreply (xprt, (xdrproc_t) xdr_outproc, out);
  if (out != NULL && !err)
    svcerr_systemerr (xprt);

  err = svc_freeargs (xprt, (xdrproc_t) xdr_inproc, (caddr_t) &in);
  if (!err)
    {
      fprintf (stderr, "svc_freeargs: Failed.\n");
      exit (EXIT_FAILURE);
    }
}
