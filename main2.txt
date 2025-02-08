#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#define numberOfPrcesses 8
#define tempFileStartingName "temp"
#define size 2000000
sem_t *lock;
int realSize = 0;
typedef struct node {
    char word[50];
    int frequency;
}node;

void splitFile(const char *fileName, int numOfChunks) {
    FILE *file = fopen(fileName, "r");

    FILE *tempFiles[numOfChunks];
    char tempFileName[50];
    for (int i = 0; i < numOfChunks; i++) {
        sprintf(tempFileName, "%s%d.txt",tempFileStartingName, i);
        tempFiles[i] = fopen(tempFileName, "w");
    }
    char line[50];
    int count = 0;
    while (fscanf(file, "%50s", line) == 1) {
        fprintf(tempFiles[count], "%s\n", line);
        count = (count + 1) % numOfChunks;
    }
    for (int k = 0; k < numOfChunks; k++) {
        fclose(tempFiles[k]);
    }
}

void procces(int fileId, node *sharedArray, int *sharedSize) {
    char tempFileName[50];
    sprintf(tempFileName, "%s%d.txt",tempFileStartingName, fileId);
    FILE *file = fopen(tempFileName, "r");


    char line[50];
    while (fscanf(file, "%50s", line) == 1) {
        int k = 0;
        for (int i = 0; i < *sharedSize; i++) {
            if (strcasecmp(sharedArray[i].word, line) == 0) {
                sem_wait(lock);
                sharedArray[i].frequency++;
                sem_post(lock);
                k = 1;
                break;
            }
        }
        if (k == 0) {
            strcpy(sharedArray[*sharedSize].word, line);
            sharedArray[*sharedSize].frequency = 1;
            (*sharedSize)++;
        }
    }
    fclose(file);
}

int compareTwoElements(const void *a, const void *b) {
    node aa = *(node *) a;
    node bb = *(node *) b;
    return bb.frequency - aa.frequency;
}

void connectAllFiles(node *sharedArray, int *sharedSize) {
    qsort(sharedArray, *sharedSize, sizeof(node), compareTwoElements);
    for (int h = 0; h < 10; h++) {
        printf("%s with frequency = %d \n", sharedArray[h].word, sharedArray[h].frequency);
    }
}
void cleanup(int numOfChunks) {
    char tempFileName[50];
    for(int i=0;i<numOfChunks;i++) {
        sprintf(tempFileName, "%s%d.txt",tempFileStartingName, i);
        remove(tempFileName);
    }
}

int main() {
    struct timeval start, end;
    gettimeofday(&start, NULL);
    lock=sem_open("semaphore",  O_CREAT,0666,1);
    splitFile("text8.txt",numberOfPrcesses);

    //shared memory creation
    int arrayId = shmget(IPC_PRIVATE, size * sizeof(node),IPC_CREAT | 0666);
    int arraySizeID = shmget(IPC_PRIVATE, sizeof(int),IPC_CREAT | 0666);
    node *sharedArray = (node *) shmat(arrayId,NULL, 0);
    int *sharedSize = (int *) shmat(arraySizeID,NULL, 0);
    *sharedSize=0;

    //child creation
    pid_t pids[numberOfPrcesses];
    for (int i = 0; i < numberOfPrcesses; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {
            procces(i,sharedArray,sharedSize);
            shmdt(sharedArray);
            shmdt(sharedSize);
            exit(0);

        }
    }
    for (int k = 0; k < numberOfPrcesses; k++) {
        waitpid(pids[k], NULL, 0);
    }
    connectAllFiles(sharedArray,sharedSize);

    //cleaning
    shmdt(sharedArray);
    shmdt(sharedSize);
    shmctl(arrayId,IPC_RMID,NULL);
    shmctl(arraySizeID,IPC_RMID,NULL);
    sem_close(lock);
    sem_unlink("semaphore");
    cleanup(numberOfPrcesses);

    //time calculation
    gettimeofday(&end, NULL);
    double time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
    printf("The execution time of the program took %f seconds\n", time);
    return 0;
}
