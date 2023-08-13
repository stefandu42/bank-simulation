#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "utils.h"
#include "utils_v1.h"

volatile sig_atomic_t end = 0;

#define BACKLOG 5

void sigint_handler (int sig) {
  char *msg = "\nFin du serveur de virements\n";
  nwrite(0, msg, strlen(msg));
  end = 1;
}

// Init the socket
int initSocketServer(int port) {
  int sockfd = ssocket();
  sbind(port, sockfd);
  slisten(sockfd, BACKLOG);
  return sockfd;
}

int main(int argc, char **argv) {
  // CTRL-C Handler
  ssigaction(SIGINT, sigint_handler);

  int port = atoi(argv[1]);
	int sockfd = initSocketServer(port);
	printf("Le serveur tourne sur le port : %i \n", port);

  // get shared memory and semaphore
	int shm_id = sshmget(SHM_KEY, NB_CLIENT * sizeof(int), IPC_CREAT | PERM);
	int* ptns = sshmat(shm_id);
  int sem_id = sem_get(SEM_KEY, 1);
  
  while (!end) {
    // Accept a new connection
    int newsockfd = accept(sockfd, NULL, NULL);

    if (end) {
      break;
    } else if (newsockfd == -1) {
      perror("Erreur de connection");
      break;
    }

    // Wait for the list of transfers
    ListVirements listVirementStruct;
    sread(newsockfd, &listVirementStruct, sizeof(listVirementStruct));

    if (end) {
      sclose(newsockfd);
      break;
    }

    int nbVirements = listVirementStruct.tailleLogique;
    int sommeMontants = 0;

    sem_down0(sem_id);
    for (int i = 0; i < nbVirements; i++) {
      // Get the information of the transfer
      int num_emeteur = listVirementStruct.listVirements[i].num_emeteur;
      int num_beneficiaire = listVirementStruct.listVirements[i].num_beneficiaire;
      int montant = listVirementStruct.listVirements[i].montant;

      /* Check if the amount of money is not negative 
         and if the receiver and the offeror are not the same and are bewteen 0 and 99 included */
      if (num_beneficiaire != num_emeteur && montant > 0 
            && num_emeteur >= 0 && num_emeteur < 100 && num_beneficiaire >= 0 && num_beneficiaire < 100) {

        // Do the transfer
        sommeMontants += montant;
        int emeteurCompte = *(ptns+num_emeteur);
	      *(ptns+num_emeteur) = emeteurCompte - montant;
        int beneficiaireCompte = *(ptns+num_beneficiaire);
        *(ptns+num_beneficiaire) = beneficiaireCompte + montant;

        // If it's only one transfer, send the balance to the client
        if (!listVirementStruct.isRecurrent) {
          swrite(newsockfd, ptns + num_emeteur, sizeof(int));
          break;
        }
      }
    }
    
    // Close the connection and the semaphore
    sem_up0(sem_id);    
    sclose(newsockfd);
  }

  // Close the shared memory and the socket
  sshmdt(ptns);
  sclose(sockfd);
}
