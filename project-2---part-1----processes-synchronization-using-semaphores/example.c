#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>

#define MAX_SLEEP 10

int *BankAccount;
sem_t *sem;

void dear_old_dad() {
    while (1) {
        sleep(rand() % 5);
        printf("Dear Old Dad: Attempting to Check Balance\n");
        
        sem_wait(sem);
        int localBalance = *BankAccount;
        int randNum = rand();
        if (randNum % 2 == 0) {
            if (localBalance < 100) {
                int deposit = rand() % 100 + 1;
                localBalance += deposit;
                printf("Dear Old Dad: Deposits $%d / Balance = $%d\n", deposit, localBalance);
            } else {
                printf("Dear Old Dad: Thinks Student has enough Cash ($%d)\n", localBalance);
            }
        } else {
            printf("Dear Old Dad: Last Checking Balance = $%d\n", localBalance);
        }
        *BankAccount = localBalance;
        sem_post(sem);
    }
}

void lovable_mom() {
    while (1) {
        sleep(rand() % MAX_SLEEP);
        printf("Lovable Mom: Attempting to Check Balance\n");
        
        sem_wait(sem);
        int localBalance = *BankAccount;
        if (localBalance <= 100) {
            int deposit = rand() % 125 + 1;
            localBalance += deposit;
            printf("Lovable Mom: Deposits $%d / Balance = $%d\n", deposit, localBalance);
        }
        *BankAccount = localBalance;
        sem_post(sem);
    }
}

void poor_student() {
    while (1) {
        sleep(rand() % 5);
        printf("Poor Student: Attempting to Check Balance\n");

        sem_wait(sem);
        int localBalance = *BankAccount;
        int randNum = rand();
        if (randNum % 2 == 0) {
            int need = rand() % 50 + 1;
            printf("Poor Student needs $%d\n", need);
            if (localBalance >= need) {
                localBalance -= need;
                printf("Poor Student: Withdraws $%d / Balance = $%d\n", need, localBalance);
            } else {
                printf("Poor Student: Not Enough Cash ($%d)\n", localBalance);
            }
        } else {
            printf("Poor Student: Last Checking Balance = $%d\n", localBalance);
        }
        *BankAccount = localBalance;
        sem_post(sem);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <Number of Parents> <Number of Students>\n", argv[0]);
        exit(1);
    }

    int num_parents = atoi(argv[1]);
    int num_students = atoi(argv[2]);

    // Initialize shared memory and semaphore
    int shm_fd = shm_open("/bank_account", O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, sizeof(int));
    BankAccount = mmap(0, sizeof(int), PROT_WRITE, MAP_SHARED, shm_fd, 0);
    *BankAccount = 0;

    sem = sem_open("/semaphore", O_CREAT, 0666, 1);

    // Create parent processes
    for (int i = 0; i < num_parents; i++) {
        if (fork() == 0) {
            if (i == 0) dear_old_dad();
            else lovable_mom();
            exit(0);
        }
    }

    // Create student processes
    for (int i = 0; i < num_students; i++) {
        if (fork() == 0) {
            poor_student();
            exit(0);
        }
    }

    // Wait for all child processes (never reached)
    for (int i = 0; i < num_parents + num_students; i++) {
        wait(NULL);
    }

    // Clean up
    munmap(BankAccount, sizeof(int));
    shm_unlink("/bank_account");
    sem_close(sem);
    sem_unlink("/semaphore");

    return 0;
}

