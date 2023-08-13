#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#include "utils.h"
#include "utils_v1.h"

volatile sig_atomic_t end = 0;

void sigint_handler(int sig) {
	end = 1;
}

void virementsReccurents(void* pipefd, void* address, void* port) {
	// Init the list of transfers
	Virement all_virements[100] = {0}; 

	// Init the logical number of the list of transfers and get the port number
	int nbVirementRecurrent = 0;
	int* ptnPort = (int*) port;

	// Get the pipe and close the unused part of pipe
	int* ptnPipeFd = pipefd;
	sclose(ptnPipeFd[1]);

	Virement virementRecu;
	while(sread(ptnPipeFd[0], &virementRecu, sizeof(Virement))) {
		if (virementRecu.num_emeteur == -2) { // Exit
			break;
		} 
		else if(virementRecu.num_emeteur == -1) { // 
			if (nbVirementRecurrent == 0) continue;				
			// Init the transfers list struct
			ListVirements listvirementStruct;
			memset(&listvirementStruct, 0, sizeof(listvirementStruct));
			listvirementStruct.tailleLogique = nbVirementRecurrent;
			listvirementStruct.isRecurrent = true;
			memcpy(listvirementStruct.listVirements, &all_virements, sizeof(all_virements));

			// Create the socket
			int sockfd = ssocket();
			sconnect(address, *ptnPort, sockfd);
			swrite(sockfd, &listvirementStruct, sizeof(listvirementStruct));
			sclose(sockfd);
		}
		else { // Add to the transfer list
			all_virements[nbVirementRecurrent++] = virementRecu;
		}
	}

	// Close the pipe and exit
	sclose(ptnPipeFd[0]);
	exit(0);
}

void childTimer(void *delay, void *pipefd) {
	// Handler for the CTRL+C
	ssigaction(SIGINT, sigint_handler);

	// Get the pipe and close the unused part of pipe
	int* ptnPipeFd = pipefd;
	sclose(ptnPipeFd[0]);


	int* ptn = (int*) delay;

	// Create a fake virement to send it in the pipe
	Virement fakeVirement = {-1, -1, -1};
	while(!end){
		sleep(*ptn);
		swrite(ptnPipeFd[1], &fakeVirement, sizeof(int));
	}

	// Close the pipe and exit
	sclose(ptnPipeFd[1]);
	exit(0);
}

int main(int argc, char **argv) {
	// Check the number of arguments
	if (argc != 5) {
		perror("Not enough args");
		exit(1);
	}

	// Get the arguments
	char* address = argv[1];
	int port = atoi(argv[2]), num = atoi(argv[3]), delay = atoi(argv[4]);

	// create the pipe
	int pipefd[2];
	spipe(pipefd);

	// Create the timer and the recurrent transfers as childs
	int childTimerId = fork_and_run2(childTimer, &delay, &pipefd);
	int childVirementsRecurrents = fork_and_run3(virementsReccurents, &pipefd, &address, &port);

	// Close the unused part of pipe  
	sclose(pipefd[0]);

	// Get the stdin
	char buffer[BUFFER_SIZE];
	while (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
		buffer[strlen(buffer) - 1] = '\0';
		const char* separator = " ";
		char* strToken = strtok (buffer, separator);

		// If no text entered
		if (strToken == NULL) {
			perror("Bad arguments");
			continue;
		}

    	if (buffer[0] == '+' || buffer[0] == '*') {
			// Get the account number and the amount from the token
			strToken = strtok (NULL, separator);
			if (strToken == NULL) {
    			perror("Bad arguments");
    			continue;
    		}
			int accountNb = atoi(strToken);
			strToken = strtok (NULL, separator);
			if (strToken == NULL) {
    			perror("Bad arguments");
    			continue;
    		}
			int amount = atoi(strToken);

			// If amount or account numbers are incorrect
			if (amount <= 0 || accountNb < 0 || accountNb >= 100) {
				printf("Le montant ou le numero de compte est incorrect\n");
				continue;
			}

			// Create a connection to the server
			int sockfd = ssocket();
			sconnect(address, port, sockfd);

			// Create the virement
			Virement virement = {num, accountNb, amount};
  		
			if (buffer[0] == '+') {
				// Create a list of recurrent transfer
				Virement listVirements[100] = {0};
				listVirements[0] = virement;
				ListVirements listvirementStruct;
				memset(&listvirementStruct, 0, sizeof(listvirementStruct));
				listvirementStruct.tailleLogique = 1;
				listvirementStruct.isRecurrent = false;
				memcpy(listvirementStruct.listVirements, &listVirements, sizeof(listVirements));

				// Send the list struct to the server
				swrite(sockfd, &listvirementStruct, sizeof(listvirementStruct));

				// Read the response of the server
				int accountSum;
				sread(sockfd, &accountSum, sizeof(int));
				printf("Balance actuelle de votre compte : %d\n", accountSum);
			} 
			else {
				// Send the transfer to the recurrent virement child
				swrite(pipefd[1], &virement, sizeof(Virement));
			}
			sclose(sockfd);
		}
		else if (buffer[0] == 'q') {
			// Send signal the the timer to stop it
			skill(childTimerId, SIGINT);
			swaitpid(childTimerId, NULL, 0);

			// Send a fake virement to the recurrent virement child to stop it
			Virement virement = {-2, -2, -2};
			swrite(pipefd[1], &virement, sizeof(Virement));
			swaitpid(childVirementsRecurrents, NULL, 0);

			// Close the pipe and exit
			sclose(pipefd[1]);
			printf("Au revoir.\n");
			exit(0);
		} 
		else {
			perror("Bad arguments");
			continue;
		}
 	}
}
