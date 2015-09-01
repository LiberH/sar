#define _XOPEN_SOURCE 700

#include "losh.h"
#include "master.h"
#include "slave.h"
#include "information.h"
#include "localisation.h"
#include "transfert.h"
#include "utils.h"
#include "xdr.h"

#include <rpc/rpc.h>
#include <rpc/pmap_clnt.h>

#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static struct master self;
struct master *master = &self;

void
master_role (void)
{
  pid_t pid;

  pid = fork ();
  if (pid == -1)
    {
      perror ("fork");
      exit (EXIT_FAILURE);
    }

  if (!pid)
    slave_role ();

  /* Init'ing the master: */
  self_addr (self.addr);
  self.slaves = NULL;
  self.slave_number = 0;
  printf ("MASTER %s\n", self.addr);

  /* Join LoSh as master: */
  join (MASTER);

  /* Run master RPC server: */
  svc_run ();
  fprintf (stderr, "svc_run: Failed.\n");
  exit (EXIT_SUCCESS);
}

void
master_dispatch (struct svc_req *request, SVCXPRT *xprt)
{
  int err;
  union {
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

    case WHOISMASTER:
      xdr_inproc  = (xdrproc_t) xdr_void;
      xdr_outproc = (xdrproc_t) xdr_master;
      function = whoismaster;
      break;

    case RECORDSLAVE:
      xdr_inproc  = (xdrproc_t) xdr_slave;
      xdr_outproc = (xdrproc_t) xdr_void;
      function = recordslave;
      break;

    case LOCATESLAVE:
      xdr_inproc  = (xdrproc_t) xdr_slave;
      xdr_outproc = (xdrproc_t) xdr_slave;
      function = locateslave;
      break;

    case INFOSLAVE:
      xdr_inproc  = (xdrproc_t) xdr_void;
      xdr_outproc = (xdrproc_t) xdr_wrapstring;
      function = infoslave;
      break;

    case STOPSLAVE:
      xdr_inproc  = (xdrproc_t) xdr_slave;
      xdr_outproc = (xdrproc_t) xdr_void;
      function = stopslave;
      break;

    case REMOVESLAVE:
      xdr_inproc  = (xdrproc_t) xdr_slave;
      xdr_outproc = (xdrproc_t) xdr_void;
      function = removeslave;
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
