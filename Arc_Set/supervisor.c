#include "supervisor.h"

static void init(void);
static void cleanUp(void);
static void checkGenerators(void);
static char * PROG_NAME;
struct buffer * buff = NULL;

int main(int argc, char *argv[])
{
    PROG_NAME = argv[0];
    if (argc != 1){
        printf("[%s] No program argument required!", PROG_NAME);
        return EXIT_FAILURE;
    }
    init();
    checkGenerators();
    cleanUp();
}

static void init(void){
    // create and/or open the shared memory object:
    int shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);
    if (shmfd == -1){
        printf("[%s] Couldn't open shared memory field descriptor!", PROG_NAME);
        exit(EXIT_FAILURE);
    }

    // set the size of the shared memory:
    if (ftruncate(shmfd, sizeof(struct buff)) < 0){
        printf("[%s] Couldn't truncate shared memory!", PROG_NAME);
        exit(EXIT_FAILURE);
    }

    buff = mmap(NULL, sizeof(*buff), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (buff == MAP_FAILED){
        printf("[%s] Couldn't map memory!", PROG_NAME);
        exit(EXIT_FAILURE);
    }
    if (close(shmfd) == -1){
        printf("[%s] Couldn't close field descriptor!", PROG_NAME);
        exit(EXIT_FAILURE);
    }

    // open semaphores
    sem_used = sem_open(SEM_USED, 0);
    if (sem_used == SEM_FAILED) {
        ERROR_EXIT("Error opening used_sem", strerror(errno));
    }
    sem_free = sem_open(SEM_FREE, 0);
    if (sem_free == SEM_FAILED) {
        ERROR_EXIT("Error opening sem_free", strerror(errno));
    }
    sem_mutex = sem_open(SEM_MUTEX, 0);
    if (sem_mutex == SEM_FAILED) {
        ERROR_EXIT("Error opening sem_mutex", strerror(errno));
    }
}

static void cleanUp(void){
    // unmap shared memory:
    if (munmap(buff, sizeof(*buff)) == -1) {
        printf("[%s] Failed to un-map shared memory!");
        exit(EXIT_FAILURE);
    }
    // remove shared memory object:
    if (shm_unlink(SHM_NAME) == -1) {
        printf("[%s] Failed to unlink shared memory!");
        exit(EXIT_FAILURE);
    }

}

static void checkGenerators(void){

}