#ifndef _UTILS_H_
#define _UTILS_H_

typedef void *(*cpyfct_t)(void *, void *);
typedef int   (*cmpfct_t)(void *, void *);

struct list_entry {
  char *data;
  struct list_entry *next;
};

void *list_add (struct list_entry **, void *, cpyfct_t);
void  list_del (struct list_entry **, void *, cmpfct_t);
void *list_find (struct list_entry **, void *, cmpfct_t);
void *list_next (struct list_entry **, int);

int  self_load (void);
void self_addr (char *);

void master_print (struct master *);

struct slave *slave_copy (struct slave *, struct slave *);
int slave_compare (struct slave *, struct slave *);
void slave_print (struct slave *);

void task_init (struct task *, char **);
struct task *task_copy (struct task *, struct task *);
int task_compare (struct task *, struct task *);
void task_print (struct task *);

#endif	/* _UTILS_H_ */
