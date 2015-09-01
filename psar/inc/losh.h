#ifndef _LOSH_H_
#define _LOSH_H_

#include <netinet/in.h>

#define LOSH  0x20000000

#define SHARING   13
#define BALANCING 37
#define THRESHOLD 42

/* MASTER */
#define MASTER       0x01
#define WHOISMASTER  1
#define RECORDSLAVE  2
#define LOCATESLAVE  3
#define INFOSLAVE    4
#define STOPSLAVE    5
#define REMOVESLAVE  6

/* SLAVE */
#define SLAVE        0x02
#define WHOISSLAVE   1
#define RECORDTASK   2
#define RETURNTASK   3
#define DELETETASK   4

struct task {
  char addr[INET_ADDRSTRLEN];
  int id;
  int argc;
  char *path;
  char **argv;
  int pid;
  int status;
};

extern int losh_type;

#endif /* _LOSH_H_ */
