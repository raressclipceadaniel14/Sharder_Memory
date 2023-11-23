#include <iostream>
#include <cstdlib>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <unistd.h>
#include <ctime>

int main() {
    std::srand(std::time(0));

    int shmid = -1;
    int* sharedMemory = nullptr;
    sem_t* semaphore;

    // Creează memorie partajată
    key_t key = ftok("/tmp", 'A');
    shmid = shmget(key, sizeof(int), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    // Atașează la memorie partajată
    sharedMemory = static_cast<int*>(shmat(shmid, nullptr, 0));
    if (sharedMemory == reinterpret_cast<int*>(-1)) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    // Creează semafor
    semaphore = sem_open("/my_semaphore", O_CREAT, 0644, 1);
    if (semaphore == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // Proces Copil
        while (true) {
            sem_wait(semaphore);
            int result = std::rand() % 2;

            if (result == 0) {
                if (*sharedMemory < 1000) {
                    (*sharedMemory)++;
                    std::cout << "Proces Copil: " << *sharedMemory << std::endl;
                }
            }

            sem_post(semaphore);
        }
    } else { // Proces Părinte
        while (true) {
            sem_wait(semaphore);
            int result = std::rand() % 2;

            if (result == 1) {
                if (*sharedMemory < 1000) {
                    (*sharedMemory)++;
                    std::cout << "Proces Părinte: " << *sharedMemory << std::endl;
                }
            }

            sem_post(semaphore);
        }
    }

    // Dezasamblează de la memorie partajată
    shmdt(sharedMemory);

    // Eliberează semaforul
    sem_close(semaphore);
    sem_unlink("/my_semaphore");

    return 0;
}
