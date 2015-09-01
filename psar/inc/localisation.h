#ifndef _LOCALISATION_H_
#define _LOCALISATION_H_

#include <rpc/rpc.h>

void locateslave_call (struct slave *);
void *locateslave (void *, struct svc_req *);

#endif /* _LOCALISATION_H_ */
