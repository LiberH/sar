#define _XOPEN_SOURCE 700

#include "losh.h"
#include "master.h"
#include "slave.h"
#include "utils.h"

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <libgen.h>

void *
list_add (struct list_entry **list, void *data, cpyfct_t cpyfct)
{
  struct list_entry *entry;
  
  entry = (struct list_entry *) malloc (sizeof (struct list_entry));
  if (entry == NULL)
    {
      perror ("malloc");
      exit (EXIT_FAILURE);
    }

  entry -> data = cpyfct (entry -> data, data);
  entry -> next = *list;
  *list = entry;

  return entry -> data;
}

void
list_del (struct list_entry **list, void *data, cmpfct_t cmpfct)
{
  struct list_entry *prev;
  struct list_entry *entry;

  prev  = *list;
  entry = *list;

  while (entry != NULL)
    {
      if (!cmpfct (entry -> data, data))
	{
	  prev -> next = entry -> next;
	  if (entry == *list)
	    *list = entry -> next;

	  free (entry);
	  return;
	}

      prev  = entry;
      entry = entry -> next;
    }

  fprintf (stderr, "list_del: Data not found.\n");
  exit (EXIT_FAILURE);
}

void *
list_next (struct list_entry **list, int restart)
{
  static struct list_entry *entry = NULL;

  if (restart)
    entry = *list;
  else
    entry = entry -> next;

  return (entry != NULL ? entry -> data : NULL);
}

void *
list_find (struct list_entry **list, void *data, cmpfct_t cmpfct)
{
  struct list_entry *entry;

  entry = *list;
  while (entry != NULL)
    {
      if (!cmpfct (entry -> data, data))
	return entry -> data;
      
      entry = entry -> next;
    }

  fprintf (stderr, "list_find: Data not found.\n");
  exit (EXIT_FAILURE);
}

void
task_init (struct task *task, char **args)
{
  int length, i;
  static int task_id = 1;
  const char *filename;

  strncpy (task -> addr, slave -> addr, INET_ADDRSTRLEN);
  task -> id = task_id++;
  for (task -> argc = 0; args[task -> argc] != NULL; task -> argc++);

  /* path */
  length = strlen (args[0]) +1;
  task -> path = (char *) malloc (sizeof (char) * length);
  if (task -> path == NULL)
    {
      perror ("malloc");
      exit (EXIT_FAILURE);
    }

  strncpy (task -> path, args[0], length);
  
  /* argv */
  task -> argv = (char **) malloc (sizeof (char *) * (task -> argc +1));
  if (task -> argv == NULL)
    {
      perror ("malloc");
      exit (EXIT_FAILURE);
    }

  filename = basename (task -> path);

  /* argv[0] */
  length = strlen (filename) +1;
  task -> argv[0] = (char *) malloc (sizeof (char) * length);
  if (task -> argv[0] == NULL)
    {
      perror ("malloc");
      exit (EXIT_FAILURE);
    }

  strncpy (task -> argv[0], filename, length);

  /* argv[i] */
  task -> argv[task -> argc] = NULL;
  for (i = 1; i < task -> argc; ++i)
    {
      length = strlen (args[i]) +1;
      task -> argv[i] = (char *) malloc (sizeof (char) * length);
      if (task -> argv[i] == NULL)
	{
	  perror ("malloc");
	  exit (EXIT_FAILURE);
	}

      strncpy (task -> argv[i], args[i], length);
    }

  task -> pid = -1;
  task -> status = 0;
}

struct task *
task_copy (struct task *dst, struct task *src)
{
  int i, length;

  dst = (struct task *) malloc (sizeof (struct task));
  if (dst == NULL)
    {
      perror ("malloc");
      exit (EXIT_FAILURE);
    }

  strncpy (dst -> addr, src -> addr, INET_ADDRSTRLEN);
  dst -> id = src -> id;
  dst -> argc = src -> argc;
  
  /* path */
  length = strlen (src -> path) +1;
  dst -> path = (char *) malloc (sizeof (char) * length);
  if (dst -> path == NULL)
    {
      perror ("malloc");
      exit (EXIT_FAILURE);
    }

  strncpy (dst -> path, src -> path, length);

  /* argv */
  dst -> argv = (char **) malloc (sizeof (char *) * (dst -> argc +1));
  if (dst -> argv == NULL)
    {
      perror ("malloc");
      exit (EXIT_FAILURE);
    }

  dst -> argv[dst -> argc] = NULL;
  for (i = 0; i < dst -> argc; ++i)
    {
      length = strlen (src -> argv[i]) +1;
      dst -> argv[i] = (char *) malloc (sizeof (char) * length);
      strncpy (dst -> argv[i], src -> argv[i], length);
    }

  dst -> pid = src -> pid;
  dst -> status = src -> status;
  return dst;
}

int
task_compare (struct task *task1, struct task *task2)
{
  int ret;
  
  ret  = strcmp (task1 -> addr, task2 -> addr);
  ret |= !(task1 -> id == task2 -> id);
  return ret;
}

void
task_print (struct task *task)
{
  int i;

  printf ("  task.addr   %s\n", task -> addr);
  printf ("  task.id     %d\n", task -> id);
  printf ("  task.argc   %d\n", task -> argc);
  printf ("  task.path   %s\n", task -> path);
  printf ("  task.argv   ");
  for (i = 0; i < task -> argc; ++i)
    printf ("%s ", task -> argv[i]);
  printf ("\n");
  printf ("  task.pid    %d\n", task -> pid);
  if (WIFEXITED (task -> status))
    printf ("  task.status %d\n", WEXITSTATUS(task -> status));
  if (WIFSIGNALED (task -> status))
    printf ("  task.signal %d\n", WTERMSIG(task -> status));
  printf ("\n");
}

struct slave *
slave_copy (struct slave *dst, struct slave *src)
{
  dst = (struct slave *) malloc (sizeof (struct slave));
  if (dst == NULL)
    {
      perror ("malloc");
      exit (EXIT_FAILURE);
    }

  strncpy (dst -> master, src -> master, INET_ADDRSTRLEN);
  strncpy (dst -> addr, src -> addr, INET_ADDRSTRLEN);
  dst -> running = src -> running;
  dst -> load = src -> load;
  dst -> task_number = src -> task_number;
  dst -> imported_task_number = src -> imported_task_number;
  return dst;
}

int
slave_compare (struct slave *slave1, struct slave *slave2)
{
  int ret;

  ret = strcmp (slave1 -> addr, slave2 -> addr);
  return ret;
}

void
slave_print (struct slave *slave)
{
  printf ("  slave.master  %s\n", slave -> master);
  printf ("  slave.addr    %s\n", slave -> addr);
  printf ("  slave.running %d\n", slave -> running);
  printf ("  slave.load    %d\n", slave -> load);
  printf ("  slave.tasks   %d\n", slave -> task_number);
  printf ("  slave.tasks   %d\n", slave -> imported_task_number);
  printf ("\n");
}

void
master_print (struct master *master)
{
  printf ("  master.addr   %s\n", master -> addr);
  printf ("  master.slaves %d\n", master -> slave_number);
  printf ("\n");
}

int
self_load (void)
{
  int   load, pid, uid, err;
  char  filename[32];
  char  line[512];
  FILE *file;
  DIR  *dir;
  struct dirent *dirent;

  /* Open /proc VFS: */
  dir = opendir ("/proc");
  if (dir == NULL)
    {
      perror ("opendir");
      exit (EXIT_FAILURE);
    }

  /* Read each entry: */
  load = 0;
  for (;;)
    {
      dirent = readdir (dir);
      if (dirent == NULL)
	break;

      /* Don't take account of not PID file names: */
      pid = strtol (dirent -> d_name, NULL, 10);
      if (!pid)
	continue;

      /* Read the status file concerning this PID: */
      sprintf (filename, "/proc/%d/status", pid);
      file = fopen (filename, "r");
      if (file == NULL)
	continue;

      /* Find out the UID: */
      fgets (line, 512, file);
      fgets (line, 512, file);
      fgets (line, 512, file);
      fgets (line, 512, file);
      fgets (line, 512, file);
      fgets (line, 512, file);
      fscanf (file, "Uid: %d", &uid);

      err = fclose (file);
      if (err == -1)
	continue;

      /* Don't take account of root processes: */
      if (!uid)
	continue;

      load++;
    }

  err = closedir (dir);
  if (err == -1)
    {
      perror ("closedir");
      exit (EXIT_FAILURE);
    }

  return load;
}

void
self_addr (char *addr)
{
  int err;
  const char *ret;
  struct ifaddrs *ifa, *ifap;
  struct sockaddr_in *sin;

  err = getifaddrs (&ifa);
  if (err == -1)
    {
      perror ("getifaddrs");
      exit (EXIT_FAILURE);
    }

  for (ifap = ifa; ifap != NULL; ifap = ifap -> ifa_next)
    {
      if (ifap -> ifa_addr -> sa_family != AF_INET)
	continue;

      sin = (struct sockaddr_in *) ifap -> ifa_addr;

#ifndef DEBUG
      if (sin -> sin_addr.s_addr == htonl (INADDR_LOOPBACK))
	continue;
#endif
      ret = inet_ntop (AF_INET,
		       (void *) &sin -> sin_addr,
		       addr, INET_ADDRSTRLEN);
      if (ret == NULL)
        {
          perror ("inet_ntop");
          exit (EXIT_FAILURE);
        }

      break;
    }

  freeifaddrs (ifa);
}
