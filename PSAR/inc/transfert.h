#ifndef _TRANSFERT_H_
#define _TRANSFERT_H_

#include <rpc/rpc.h>

void execute (char **);

void  recordtask_call (struct slave *, struct task *);
void *recordtask (void *, struct svc_req *);

void  returntask_call (struct slave *, struct task *);
void *returntask (void *, struct svc_req *);

#endif /* _TRANSFERT_H_ */
