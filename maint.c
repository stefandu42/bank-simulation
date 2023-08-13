#include <stdlib.h>
#include <unistd.h>

#include "utils.h"
#include "utils_v1.h"

int main(int argc, char **argv) {
  if (argc < 2) {
  	perror("Not enough args");
  	exit(1);
  }

  int type = atoi(argv[1]);
  if (type == 1) {
	sem_create(SEM_KEY, 1, PERM, 1);
	sshmget(SHM_KEY, NB_CLIENT * sizeof(int), IPC_CREAT | PERM);
  } else if (type == 2) {
    int sem_id = sem_get(SEM_KEY, 1);
  	int shm_id = sshmget(SHM_KEY, NB_CLIENT * sizeof(int), IPC_CREAT | PERM);
	  sshmdelete(shm_id);
  	sem_delete(sem_id);
  } else if (type == 3) {
  	if (argc >= 3) {
  		int sem_id = sem_get(SEM_KEY, 1);
  		int opt = atoi(argv[2]);
  		sem_down0(sem_id);
  		printf("Mémoire réservée pour %d secondes\n", opt);
  		sleep(opt);
  		sem_up0(sem_id);
  	} else {
		perror("Need opt argument");
   		exit(1);
  	}
  } else {
  	perror("Bad argument");
   	exit(1);
  }
}
