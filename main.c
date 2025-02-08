#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <semaphore.h>
#include <fcntl.h>
#include <pthread.h>
#define numberOfThreads 8
#define tempFileStartingName "temp"
#define size 2000000
sem_t *lock;

typedef struct node {
    char word[50];
    int frequency;
} node;
node * sharedArray;
int sharedSize = 0;
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

void procces(void *args) {
    int fileId = *(int *)args;
    free(args);

    char tempFileName[50];
    sprintf(tempFileName, "%s%d.txt",tempFileStartingName, fileId);
    FILE *file = fopen(tempFileName, "r");


    char line[50];
    while (fscanf(file, "%50s", line) == 1) {
        int k = 0;
        for (int i = 0; i < sharedSize; i++) {
            if (strcasecmp(sharedArray[i].word, line) == 0) {
                sem_wait(lock);
                sharedArray[i].frequency++;
                sem_post(lock);
                k = 1;
                break;
            }
        }
        if (k == 0) {
            strcpy(sharedArray[sharedSize].word, line);
            sharedArray[sharedSize].frequency = 1;
            sharedSize++;
        }
    }
    fclose(file);
    pthread_exit(NULL);
}

int compareTwoElements(const void *a, const void *b) {
    node aa = *(node *) a;
    node bb = *(node *) b;
    return bb.frequency - aa.frequency;
}

void connectAllFiles(node *sharedArray) {
    qsort(sharedArray, sharedSize, sizeof(node), compareTwoElements);
    for (int h = 0; h < 10; h++) {
        printf("%s with frequency = %d \n",sharedArray[h].word, sharedArray[h].frequency);
    }
}

void cleanup(int numOfChunks) {
    char tempFileName[50];
    for (int i = 0; i < numOfChunks; i++) {
        sprintf(tempFileName, "%s%d.txt",tempFileStartingName, i);
        remove(tempFileName);
    }
}

int main() {
    struct timeval start, end;
    gettimeofday(&start, NULL);
    lock = sem_open("semaphore", O_CREAT, 0666, 1);
    splitFile("input.txt",numberOfThreads);
    sharedArray = malloc(size*sizeof(node));

    //thread creation
    pthread_t threads[numberOfThreads];
    for (int i = 0; i < numberOfThreads; i++) {
        int *fileId =malloc(sizeof(int));
        *fileId=i;
        pthread_create(&threads[i],NULL,procces,fileId);

    }
    for (int k = 0; k < numberOfThreads; k++) {
        pthread_join(threads[k],NULL);
    }
    connectAllFiles(sharedArray);

    //cleaning
    sem_close(lock);
    sem_unlink("semaphore");
    cleanup(numberOfThreads);

    //time calculation
    gettimeofday(&end, NULL);
    double time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
    printf("The execution time of the program took %f seconds\n", time);
    return 0;
}
