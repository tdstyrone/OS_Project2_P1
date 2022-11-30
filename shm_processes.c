// Tyrone Smith + Deanté Taylor

#include  <stdio.h>
#include  <stdlib.h>
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>

void Poor_Student_Process(int[], sem_t * , int);
void Dear_Old_Dad_Process(int[], sem_t * );
void Loveable_Mom_Process(int SharedMem[], sem_t * mutex);

int main(int argc, char * argv[]) {
  if (argc < 3) {
    printf("Expected parameters (num_parents[1-2]) (num_children[>=1])\n");
    exit(1);
  }
  int num_parents = atoi(argv[1]);
  if (num_parents != 1 && num_parents !=2){
    printf("num_parents must be 1 or 2\n");
    exit(1);
  }
  int num_childs = atoi(argv[2]);
  if (num_childs <=0){
    printf("num_children must be atleast 1\n");
  }
  int ShmID;
  int i;
  int * ShmPTR;
  int total= num_childs + num_parents;
  pid_t pids[total];
  sem_t * mutex;

  ShmID = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
  if (ShmID < 0) {
    printf("*** shmget error ***\n");
    exit(1);
  }

  ShmPTR = (int * ) shmat(ShmID, NULL, 0);
  if ( * ShmPTR == -1) {
    printf("*** shmat error ***\n");
    exit(1);
  }

  ShmPTR[0] = 0; //  BankAccount

  if ((mutex = sem_open("semaphore", O_CREAT, 0644, 1)) == SEM_FAILED) {
    perror("semaphore initilization");
    exit(1);
  }

  for (i = 0; i < total; i++){
    pids[i] = fork();
    if (pids[i] < 0) {
      printf("*** fork error ***\n");
      exit(1);
    }
    else if (pids[i] == 0) {
      if (i == 0){
        Dear_Old_Dad_Process(ShmPTR, mutex);
      }
      else if (i == 1 && num_parents == 2) {
        Loveable_Mom_Process(ShmPTR, mutex);
      }
      else {
        Poor_Student_Process(ShmPTR, mutex, i - num_parents + 1);
      }
      exit(0);
    }
  }
  
  for (i = 0; i < total; i++){
   wait(NULL); 
  }
  
}

void Poor_Student_Process(int SharedMem[], sem_t * mutex, int id) {
  while (1) {
    sleep(rand() % 5);
    printf("Poor Student #%d: Attempting to Check Balance\n", id);
    sem_wait(mutex);
    int account = SharedMem[0];
    int balance = rand() % 50;
    int r = rand();
    if (r % 2 == 0) {
      if (balance <= account) {
        account -= balance;
        printf("Poor Student #%d: Withdraws $%d / Balance = $%d\n",id, balance, account);
      } else {
        printf("Poor Student #%d: Not Enough Cash ($%d)\n",id, account);
      }
    } else {
      printf("Poor Student #%d: Last Checking Balance = $%d\n",id, account);
    }

    SharedMem[0] = account;
    SharedMem[1] = 0;
    sem_post(mutex);
  }

}

void Dear_Old_Dad_Process(int SharedMem[], sem_t * mutex) {
  while (1) {
    sleep(rand() % 5);
    printf("Dear Old Dad: Attempting to Check Balance\n");
    sem_wait(mutex);
    int account = SharedMem[0];

    int r = rand();
    if (r % 2 == 0) {
      if (account < 100) {
        int balance = rand() % 100;
        if (balance % 2 == 0) {
          account += balance;
          printf("Dear old Dad: Deposits $%d / Balance = $%d\n", balance, account);
        } else {
          printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", account);
        }
      } else {
        printf("Dear Old Dad: Last Checking Balance = $%d\n", account);
      }
    }

    SharedMem[0] = account;
    sem_post(mutex);
  }
}

void Loveable_Mom_Process(int SharedMem[], sem_t * mutex) {

  while (1) {
    sleep(rand() % 5);
    printf("Loveable Mom: Attempting to Check Balance\n");
    sem_wait(mutex);
    int account = SharedMem[0];

    if (account < 100) {
      int balance = rand() % 125;
      account += balance;
      printf("Lovable Mom: Deposits $%d / Balance = $%d\n", balance, account);

    }
    SharedMem[0] = account;
    sem_post(mutex);
  }
}