#define _XOPEN_SOURCE 700

#include "losh.h"
#include "slave.h"
#include "master.h"
#include "information.h"
#include "transfert.h"
#include "ihm.h"
#include "utils.h"

#include <netinet/in.h>
#include <string.h>

char **
getARGV (char *commande)
{
  int i = 0;
  int k;
  int trouver = 0;
  int j = 0;
  int compteur = 0;
  char *tampo;
  char **resultat;

  for(i = 0 ; i < strlen(commande); i++){
    if(commande[i] == ' '){
      trouver = 0;
       
      continue;
    }
    
    if(trouver == 0){
      compteur++;
      trouver = 1;
    }  
  }

 
  resultat = malloc(sizeof(char *) * (compteur + 1));

  i  = 0;

  tampo = malloc(sizeof(char) * TSIZE);

  for(k = 0; k < compteur; k++){
    j = 0;
    trouver = 0;
    for(i = i ; i < strlen(commande); i++){
      if(j < TSIZE ){

        if(commande[i] == ' '){
          if(trouver == 1)
            break;   

          continue;
        }
   
        trouver = 1;
        tampo[j++] = commande[i]; 
         
      }
    }

    tampo[j] = '\0';

    resultat[k] = malloc(sizeof(char) * TSIZE);
    strcpy(resultat[k], tampo); 
    free(tampo);
    tampo = malloc(sizeof(char) * TSIZE);

  }
  
  free(tampo);

  resultat[compteur] = NULL;

  return resultat;
}

void
entrerCommande ()
{
  char buf[CMD_BUFSIZE]; 
  char *cmd;
  char *args;
  char *commande;
  struct slave peer;

  for (;;)
    {
      memset ((void *) buf, 0, sizeof (char) * CMD_BUFSIZE);
      fflush(stdout);
      printf("> ");
      fflush(stdout);
      
      fgets((char*) &buf, CMD_BUFSIZE, stdin);
      cmd = strtok((char*) &buf ," \n\t");
      args = strtok(NULL, "\n\t");
      
      if (cmd == NULL)
	continue;

      if (!strcmp (CMDNAME_STAT, cmd))
	{
	  if (args == NULL ||
	      !strcmp (args, "*"))
	    {
	      infoslave_call ();
	    }
	  else
	    {
	      strncpy (peer.addr, args, INET_ADDRSTRLEN);
	      if (whoisslave_call (&peer))
		{
		  printf ("SLAVE %s\n", peer.addr);
		  printf ("  load           %d\n", peer.load);
		  printf ("  exported tasks %d\n", peer.task_number);
		  printf ("  imported tasks %d\n", peer.imported_task_number);
		  printf ("\n");
		}
	    }
	}
      else if (!strcmp (CMDNAME_EXEC, cmd) &&
	       args != NULL)
	{
	  commande = malloc (strlen (args));
	  strcpy (commande, args);
	  execute (getARGV (commande));
	}
      else if (!strcmp (CMDNAME_KILL, cmd))
	{
	  leave (SLAVE, FALSE);
	  return;
	
	}
      else if (!strcmp(CMDNAME_QUIT, cmd))
	{
	  leave (SLAVE, TRUE);
	  return;  
	}
      else if (!strcmp(CMDNAME_HELP, cmd))
	{
	  printf("%s  Executer un programme\n", CMDNAME_EXEC);
	  printf("%s  Quitter l'application après terminaison des tâches\n", CMDNAME_QUIT);
	  printf("%s  Quitter l'application sans terminaison des tâches\n", CMDNAME_KILL);
	  printf("%s Affiche l'état du système\n", CMDNAME_STAT);
	  printf("%s  Affiche cette aide\n", CMDNAME_HELP);
	}
      else
	{
	  printf ("Commande invalide\n");
	  printf ("(%s pour obtenir l'aide)\n", CMDNAME_HELP);
	}
    }
}
