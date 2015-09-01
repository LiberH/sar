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
#include <arpa/inet.h>

#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int _slave_count;
static int iammaster (caddr_t, struct sockaddr_in *);
static int iamslave  (caddr_t, struct sockaddr_in *);
static int deleted   (caddr_t, struct sockaddr_in *);

/* TODO
 *  Kill slave -> task on quit?
 */

/* Local */

void
join (int type)
{
  int err;
  SVCXPRT *xprt;

  xprt = svcudp_create (RPC_ANYSOCK);
  if (xprt == NULL)
    {
      fprintf (stderr, "svcudp_create: Failed.\n");
      exit (EXIT_FAILURE);
    }

  switch (type)
    {
    case MASTER:
      /* Create master RPC server: */
      pmap_unset (LOSH, MASTER);
      err = svc_register (xprt, LOSH, MASTER, master_dispatch, IPPROTO_UDP);
      if (!err)
	{
	  fprintf (stderr, "svc_register: Failed.\n");
	  exit (EXIT_FAILURE);
	}
      break;

    case SLAVE:
      /* Create slave RPC server: */
      pmap_unset (LOSH, SLAVE);
      err = svc_register (xprt, LOSH, SLAVE, slave_dispatch, IPPROTO_UDP);
      if (!err)
	{
	  fprintf (stderr, "svc_register: Failed.\n");
	  exit (EXIT_FAILURE);
	}

      /* Join LoSh master: */
      whoismaster_broadcast ();
      recordslave_call ();
      break;
    }
}

void
leave (int type, int wait)
{
  struct task *task;

  switch (type)
    {
    case MASTER:
      pmap_unset (LOSH, MASTER);
      break;

    case SLAVE:
      switch (wait)
	{
	case TRUE:
	  stopslave_call ();
	  while (slave -> task_number ||
		 slave -> imported_task_number);
	  break;

	case FALSE:
	  deletetask_broadcast (slave);
	  task = NULL;
	  while (TRUE)
	    {
	      /* Foreach imported task: */
	      task = (struct task *) list_next (&slave -> imported_tasks,
						(task == NULL));
	      if (task == NULL)
		break;

	      /* Kill it: */
	      if (task -> pid != -1)
		{
		  kill (task -> pid, SIGKILL);
#ifdef DEBUG
		  printf ("SLAV (info) killed task %d\n", (int) task -> pid);
#endif
		}
	    }

	  break;
	}

      removeslave_call ();
      pmap_unset (LOSH, SLAVE);
      break;
    }
}

/* WHOISMASTER */

void
whoismaster_broadcast (void)
{
  int err;
  struct master master;

#ifdef DEBUG
  printf ("SLAV [call] whoismaster\n");
#endif
  err = clnt_broadcast (LOSH, MASTER, WHOISMASTER,
			(xdrproc_t) xdr_void,   (char *) NULL,
                        (xdrproc_t) xdr_master, (char *) &master,
			iammaster);
  if (err)
    {
      fprintf (stderr, "clnt_broadcast: Not replied.\n");
      exit (EXIT_FAILURE);
    }

#ifdef DEBUG
  printf ("SLAV [resp] whoismaster\n");
  master_print (&master);
#endif
}

void
*whoismaster (void *arg, struct svc_req *request)
{
#ifdef DEBUG
  printf ("MAST [exec] iammaster\n");
#endif
  return (void *) master;
}

bool_t
iammaster (caddr_t arg, struct sockaddr_in *sin)
{
  struct master *master;

  master = (struct master *) arg;
  strcpy (slave -> master, master -> addr);
  return TRUE;
}

/* WHOISSLAVE */

void
whoisslave_broadcast ()
{
  int err;
  struct slave *temp;
  struct slave slave;

#ifdef DEBUG
  printf ("MAST [call] whoisslave\n");
#endif
  _slave_count = 0;

  /* Unset all known loads: */
  temp = NULL;
  while (TRUE)
    {
      temp = (struct slave *) list_next (&master -> slaves,
					 (temp == NULL));
      if (temp == NULL)
	break;

      temp -> load = -1;
    }

  err = clnt_broadcast (LOSH, SLAVE, WHOISSLAVE,
                        (xdrproc_t) xdr_void,  (char *) NULL,
			(xdrproc_t) xdr_slave, (char *) &slave,
                        iamslave);
  if (err)
    {
      temp = NULL;
      while (TRUE)
	{
	  temp = (struct slave *) list_next (&master -> slaves,
					     (temp == NULL));
	  if (temp == NULL)
	    break;
	  
	  if (temp -> load == -1)
	    {
#ifdef DEBUG
	      printf ("Breakdown %s\n", temp -> addr);
#endif
	      deletetask_broadcast (temp);
	      list_del (&master -> slaves, (void *) temp,
			(cmpfct_t) task_compare);
	      master -> slave_number--;
	    }
	}

#ifdef DEBUG
      master_print (master);
#endif
    }

#ifdef DEBUG
  printf ("MAST [resp] whoisslave\n");
  temp = NULL;
  while (TRUE)
    {
      temp = (struct slave *) list_next (&master -> slaves,
					 (temp == NULL));
      if (temp == NULL)
	break;

      slave_print (temp);
    }
#endif
}

int
whoisslave_call (struct slave *peer)
{
  int err;
  CLIENT *clnt;
  struct timeval timeout;

#ifdef DEBUG
  printf ("**** [call] whoislave\n");
#endif

  clnt = clnt_create (peer -> addr, LOSH, SLAVE, "udp");
  if (clnt == NULL)
    {
      fprintf (stderr, "whoisslave_call: Peer not found.\n");
      return FALSE;
    }

  timeout.tv_sec  = 10;
  timeout.tv_usec =  0;
  err = clnt_call (clnt, WHOISSLAVE,
                   (xdrproc_t) xdr_void,  (char *) NULL,
                   (xdrproc_t) xdr_slave, (char *) peer,
                   timeout);
  if (err)
    {
      fprintf (stderr, "clnt_call: Failed.\n");
      exit (EXIT_FAILURE);
    }

  clnt_destroy (clnt);
#ifdef DEBUG
  printf ("**** [resp] whoislave\n");
#endif
  return TRUE;
}

void
*whoisslave (void *arg, struct svc_req *request)
{
#ifdef DEBUG
  printf ("SLAV [exec] iamslave\n");
#endif
  slave -> load = self_load ();
  return (void *) slave;
}

bool_t
iamslave (caddr_t arg, struct sockaddr_in *sin)
{
  struct slave *slave, *temp;

  slave = (struct slave *) arg;
  temp  = (struct slave *) list_find (&master -> slaves,
				      (void *) slave,
				      (cmpfct_t) slave_compare);
  if (temp -> load != -1)
    return 0;
  
  temp -> load = slave -> load;
  return (++_slave_count == master -> slave_number);
}

/* DELETETASK */

void
deletetask_broadcast (struct slave *slave)
{
  int err;

#ifdef DEBUG
  printf ("**** [call] deletetask\n");
#endif
  err = clnt_broadcast (LOSH, SLAVE, DELETETASK,
			(xdrproc_t) xdr_slave, (char *) slave,
                        (xdrproc_t) xdr_void,  (char *) NULL,
			deleted);
  if (err)
    {
      fprintf (stderr, "deletetask_broadcast: No reply.\n");
      return;
    }

#ifdef DEBUG
  printf ("**** [resp] deletetask\n");
#endif
}

void
*deletetask (void *arg, struct svc_req *request)
{
  static caddr_t reply;
  struct slave *peer;
  struct task *task;

  peer = (struct slave *) arg;
  if (!strcmp (peer -> addr, slave -> addr))
    {
      reply = NULL;
      return (void *) reply;
    }

#ifdef DEBUG
  printf ("SLAV [exec] deletetask\n");
#endif

  task = NULL;
  while (TRUE)
    {
      task = (struct task *) list_next (&slave -> imported_tasks,
					(task == NULL));
      if (task == NULL)
	break;
      
      /* If same addr, kill it: */
      if (!strcmp (task -> addr, peer -> addr) &&
	  task -> pid != -1)
	{
	  kill (task -> pid, SIGKILL);
#ifdef DEBUG
	  printf ("SLAV (info) killed task %d\n", (int) task -> pid);
#endif
	} 
    }

  reply = NULL;
  return (void *) reply;
}

bool_t
deleted (caddr_t arg, struct sockaddr_in *sin)
{
  return TRUE;
}

/* RECORDSLAVE */

void
recordslave_call (void)
{
  int err;
  CLIENT *clnt;
  struct timeval timeout;

#ifdef DEBUG
  printf ("SLAV [call] recordslave\n");
#endif
  slave -> load = self_load ();
  clnt = clnt_create (slave -> master, LOSH, MASTER, "udp");
  if (clnt == NULL)
    {
      fprintf (stderr, "clnt_create: Failed.\n");
      exit (EXIT_FAILURE);
    }

  timeout.tv_sec  = 10;
  timeout.tv_usec =  0;
  err = clnt_call (clnt, RECORDSLAVE,
                   (xdrproc_t) xdr_slave, (char *) slave,
                   (xdrproc_t) xdr_void,  (char *) NULL,
                   timeout);
  if (err)
    {
      fprintf (stderr, "clnt_call: Failed.\n");
      exit (EXIT_FAILURE);
    }

  clnt_destroy (clnt);
#ifdef DEBUG
  printf ("SLAV [resp] recordslave\n");
#endif
}

void *
recordslave (void *arg, struct svc_req *request)
{
  static caddr_t reply;
  struct slave *slave;

  slave = (struct slave *) arg;
  list_add (&master -> slaves, (void *) slave, (cpyfct_t) slave_copy);
  master -> slave_number++;

#ifdef DEBUG
  printf ("MAST [exec] recordslave\n");
  master_print (master);
#endif

  reply = NULL;
  return (void *) &reply;
}

/* INFOSLAVE */

void
infoslave_call (void)
{
  int err;
  static char *out;
  CLIENT *clnt;
  struct timeval timeout;

#ifdef DEBUG
  printf ("SLAV [call] infoslave\n");
#endif
  clnt = clnt_create (slave -> master, LOSH, MASTER, "udp");
  if (clnt == NULL)
    {
      fprintf (stderr, "clnt_create: Failed.\n");
      exit (EXIT_FAILURE);
    }

  out = NULL;
  timeout.tv_sec  = 10;
  timeout.tv_usec =  0;
  err = clnt_call (clnt, INFOSLAVE,
                   (xdrproc_t) xdr_void, (char *) NULL,
                   (xdrproc_t) xdr_wrapstring, (char *) &out,
                   timeout);
  if (err)
    {
      fprintf (stderr, "clnt_call: Failed.\n");
      exit (EXIT_FAILURE);
    }

  printf ("%s", out);
  clnt_destroy (clnt);
#ifdef DEBUG
  printf ("SLAV [resp] infoslave\n");
#endif
}

void *
infoslave (void *arg, struct svc_req *request)
{
  static char *info = NULL;
  struct slave *slave;

#ifdef DEBUG
  printf ("MAST [exec] infoslave\n");
#endif

  if (info != NULL)
    free (info);

  info = (char *) malloc (sizeof (char) * master -> slave_number * 256);
  if (info == NULL)
    {
      perror ("malloc");
      exit (EXIT_FAILURE);
    }

  slave = NULL;
  memset ((void *) info, 0, sizeof (char) * master -> slave_number * 256);
  while (TRUE)
    {
      slave = (struct slave *) list_next (&master -> slaves,
					  (slave == NULL));
      if (slave == NULL)
	break;

      whoisslave_call (slave);

      sprintf (info + strlen (info), "SLAVE %s\n", slave -> addr);
      sprintf (info + strlen (info), "  load           %d\n", slave -> load);
      sprintf (info + strlen (info), "  exported tasks %d\n", slave -> task_number);
      sprintf (info + strlen (info), "  imported tasks %d\n", slave -> imported_task_number);
      sprintf (info + strlen (info), "\n");
    }

  return (void *) &info;
}

/* STOPSLAVE */

void
stopslave_call (void)
{
  int err;
  CLIENT *clnt;
  struct timeval timeout;

#ifdef DEBUG
  printf ("SLAV [call] stopslave\n");
#endif
  clnt = clnt_create (slave -> master, LOSH, MASTER, "udp");
  if (clnt == NULL)
    {
      fprintf (stderr, "clnt_create: Failed.\n");
      exit (EXIT_FAILURE);
    }

  timeout.tv_sec  = 10;
  timeout.tv_usec =  0;
  err = clnt_call (clnt, STOPSLAVE,
                   (xdrproc_t) xdr_slave, (char *) slave,
                   (xdrproc_t) xdr_void,  (char *) NULL,
                   timeout);
  if (err)
    {
      fprintf (stderr, "clnt_call: Failed.\n");
      exit (EXIT_FAILURE);
    }

  clnt_destroy (clnt);
#ifdef DEBUG
  printf ("SLAV [resp] stopslave\n");
#endif
}

void *
stopslave (void *arg, struct svc_req *request)
{
  static caddr_t reply;
  struct slave *slave;

  slave = (struct slave *) arg;
  slave = (struct slave *) list_find (&master -> slaves,
				      (void *) slave,
				      (cmpfct_t) slave_compare);
  slave -> running = FALSE;

#ifdef DEBUG
  printf ("MAST [exec] stopslave\n");
  slave_print (slave);
#endif

  reply = NULL;
  return (void *) &reply;
}

/* REMOVESLAVE */

void
removeslave_call (void)
{
  int err;
  CLIENT *clnt;
  struct timeval timeout;

#ifdef DEBUG
  printf ("SLAV [call] removeslave\n");
#endif
  clnt = clnt_create (slave -> master, LOSH, MASTER, "udp");
  if (clnt == NULL)
    {
      fprintf (stderr, "clnt_create: Failed.\n");
      exit (EXIT_FAILURE);
    }

  timeout.tv_sec  = 10;
  timeout.tv_usec =  0;
  err = clnt_call (clnt, REMOVESLAVE,
                   (xdrproc_t) xdr_slave, (char *) slave,
                   (xdrproc_t) xdr_void,  (char *) NULL,
                   timeout);
  if (err)
    {
      fprintf (stderr, "clnt_call: Failed.\n");
      exit (EXIT_FAILURE);
    }

  clnt_destroy (clnt);
#ifdef DEBUG
  printf ("SLAV [resp] removeslave\n");
#endif
}

void *
removeslave (void *arg, struct svc_req *request)
{
  static caddr_t reply;
  struct slave *slave;

  slave = (struct slave *) arg;
  list_del (&master -> slaves, (void *) slave, (cmpfct_t) slave_compare);
  master -> slave_number--;

#ifdef DEBUG
  printf ("MAST [exec] removeslave\n");
  master_print (master);
#endif

  reply = NULL;
  return (void *) &reply;
}
