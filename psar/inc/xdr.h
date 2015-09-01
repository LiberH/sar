#ifndef _XDR_H_
#define _XDR_H_

#include <rpc/rpc.h>

bool_t xdr_slave (XDR *, struct slave *);
bool_t xdr_master (XDR *, struct master *);
bool_t xdr_task (XDR *, struct task *);

#endif /* _XDR_H_ */
