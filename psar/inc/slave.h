#ifndef _SLAVE_H_
#define _SLAVE_H_

#include <pthread.h>
#include <rpc/rpc.h>
#include <netinet/in.h>

struct slave {
  char master[INET_ADDRSTRLEN];
  char addr[INET_ADDRSTRLEN];
  int running;
  int load;

  struct list_entry *tasks;
  struct list_entry *imported_tasks;
  int task_number;
  int imported_task_number;
};

extern struct slave *slave;
void slave_role (void);
void slave_dispatch (struct svc_req *, SVCXPRT *);

#endif	/* _SLAVE_H_ */
