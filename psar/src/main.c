#define _XOPEN_SOURCE 700

#include "losh.h"
#include "master.h"
#include "slave.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int losh_type = SHARING;

int
main (int argc, char **argv)
{
  if (argc == 2 &&
      !strcmp (argv[1], "--master"))
    master_role ();

  if (argc == 1)
    slave_role ();

  fprintf (stderr, "Usage:\n  %s [--master]\n", argv[0]);
  return EXIT_FAILURE;
}
