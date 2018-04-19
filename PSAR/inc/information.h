#ifndef _INFORMATION_H_
#define _INFORMATION_H_

#include <rpc/rpc.h>

void join  (int);
void leave (int, int);

void  whoismaster_broadcast (void);
void *whoismaster (void *, struct svc_req *);

void  whoisslave_broadcast (void);
int   whoisslave_call (struct slave *);
void *whoisslave (void *, struct svc_req *);

void  deletetask_broadcast (struct slave *);
void *deletetask (void *, struct svc_req *);

void  recordslave_call (void);
void *recordslave (void *, struct svc_req *);

void  infoslave_call (void);
void *infoslave (void *, struct svc_req *);

void  stopslave_call (void);
void *stopslave (void *, struct svc_req *);

void  removeslave_call (void);
void *removeslave (void *, struct svc_req *);

#endif /* _INFORMATION_H_ */
