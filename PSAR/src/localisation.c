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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* LOCATESLAVE */

void
locateslave_call (struct slave *peer)
{
  int err;
  CLIENT *clnt;
  struct timeval timeout;

#ifdef DEBUG
  printf ("SLAV [call] locateslave\n");
#endif
  clnt = clnt_create (slave -> master, LOSH, MASTER, "udp");
  if (clnt == NULL)
    {
      fprintf (stderr, "clnt_create: Failed.\n");
      exit (EXIT_FAILURE);
    }

  timeout.tv_sec  = 10;
  timeout.tv_usec =  0;
  err = clnt_call (clnt, LOCATESLAVE,
                   (xdrproc_t) xdr_slave, (char *) slave,
                   (xdrproc_t) xdr_slave, (char *) peer,
                   timeout);
  if (err)
    {
      fprintf (stderr, "clnt_call: Failed.\n");
      exit (EXIT_FAILURE);
    }

  clnt_destroy (clnt);
#ifdef DEBUG
  printf ("SLAV [resp] locateslave\n");
  slave_print (peer);
#endif
}

void
*locateslave (void *arg, struct svc_req *request)
{
  struct slave *peer, *temp;

#ifdef DEBUG
  printf ("MAST [exec] locateslave\n");
#endif
  whoisslave_broadcast ();

  switch (losh_type)
    {
    case SHARING:
    case BALANCING:

      /* Min val: */
      peer = NULL;
      temp = NULL;
      while (TRUE)
	{
	  temp = (struct slave *) list_next (&master -> slaves,
					     (temp == NULL));
	  if (temp == NULL)
	    break;

	  if (!temp -> running)
	    continue;

	  if (peer == NULL ||
	      temp -> load < peer -> load)
	    peer = temp;
	}

      break;
    }

  return (void *) peer;
}

/* Mean val:
int mean;

mean = 0;
temp = (struct slave *) list_next (&master -> slaves, TRUE);
for (;;)
  {
    temp = (struct slave *) list_next (&master -> slaves, FALSE);
    if (temp == NULL)
      break;
    
    mean += temp -> load;
  }
*/
