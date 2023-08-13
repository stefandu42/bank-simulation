#include "utils.h"
#include "utils_v1.h"
#include <stdlib.h>

#define PERM 0666

int livreComptes[1000] = {0};

int main(int argc, char **argv) {
	//if there is not all the args
	if(argc != 3){
		exit(1);
	}
	// convert args to integers
	int account = atoi(argv[1]);
	int sumMoney = atoi(argv[2]);
	
	// get shared memory
	int shm_id = sshmget(SHM_KEY, NB_CLIENT * sizeof(int), IPC_CREAT | PERM);
	int* ptns = sshmat(shm_id);


	// make the operation 
	int sem_id = sem_get(SEM_KEY, 1);
	sem_down0(sem_id);
	int actualAccount = *(ptns+account);
	*(ptns+account) = actualAccount + sumMoney;
	printf("Nouveau solde : %d\n", *(ptns+account));
	sshmdt(ptns);
	sem_up0(sem_id);
}
