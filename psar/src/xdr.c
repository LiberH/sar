#define _XOPEN_SOURCE 700

#include "losh.h"
#include "slave.h"
#include "master.h"
#include "xdr.h"

#include <netinet/in.h>

bool_t
xdr_master (XDR *xdrs, struct master *master)
{
  if (!xdr_vector (xdrs, (char *) &master -> addr, INET_ADDRSTRLEN,
		   sizeof (char), (xdrproc_t) xdr_char))
    return FALSE;
  if (!xdr_int (xdrs, &master -> slave_number)) return FALSE;

  return TRUE;
}

bool_t
xdr_slave (XDR *xdrs, struct slave *slave)
{
  if (!xdr_vector (xdrs, (char *) &slave -> master, INET_ADDRSTRLEN,
		   sizeof (char), (xdrproc_t) xdr_char))
    return FALSE;
  if (!xdr_vector (xdrs, (char *) &slave -> addr, INET_ADDRSTRLEN,
		   sizeof (char), (xdrproc_t) xdr_char))
    return FALSE;
  if (!xdr_int (xdrs, &slave -> running)) return FALSE;
  if (!xdr_int (xdrs, &slave -> load)) return FALSE;
  if (!xdr_int (xdrs, &slave -> task_number)) return FALSE;
  if (!xdr_int (xdrs, &slave -> imported_task_number)) return FALSE;

  return TRUE;
}

bool_t
xdr_task (XDR *xdrs, struct task *task)
{
  if (!xdr_vector (xdrs, (char *) &task -> addr, INET_ADDRSTRLEN,
		   sizeof (char), (xdrproc_t) xdr_char))
    return FALSE;

  if (!xdr_int (xdrs, &task -> id)) return FALSE;
  if (!xdr_int (xdrs, &task -> argc)) return FALSE;
  if (!xdr_wrapstring (xdrs, &task -> path)) return FALSE;
  if (!xdr_array (xdrs, (char **) &task -> argv,
		  (u_int *) &task -> argc, ~0, sizeof (char *),
		  (xdrproc_t) xdr_wrapstring))
    return FALSE;  

  if (!xdr_int (xdrs, &task -> pid)) return FALSE;
  if (!xdr_int (xdrs, &task -> status)) return FALSE;

  return TRUE;
}
