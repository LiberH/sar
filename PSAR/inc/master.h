#ifndef _MASTER_H_
#define _MASTER_H_

#include <rpc/rpc.h>
#include <netinet/in.h>

struct master {
  char addr[INET_ADDRSTRLEN];

  struct list_entry *slaves;
  int slave_number;
};

extern struct master *master;
void master_role (void);
void master_dispatch (struct svc_req *, SVCXPRT *);

#endif	/* _MASTER_H_ */
