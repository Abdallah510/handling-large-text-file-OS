#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
//abdallah
int size=2000000;
int realsize=0;
typedef struct node {
    char word[50];
    int frequency;
} node;
void insert(const char *word,node *array) {
    for(int i =0;i<realsize;i++) {
        if(strcmp(array[i].word,word)==0) {
            array[i].frequency++;
            return;
        }
    }
    strcpy(array[realsize].word,word);
    array[realsize].frequency=1;
    realsize++;
}
int compareTwoElements(const void *a,const void *b) {
    node aa= *(node *)a;
    node bb= *(node *)b;
    return bb.frequency - aa.frequency;
}
void topTinFrequentWords(node *array) {

    qsort(array,realsize,sizeof(node),compareTwoElements);
    for(int k=0;k<10;k++) {
        printf("%s with frequency = %d \n",array[k].word,array[k].frequency);
    }
}

int main() {
    clock_t start,end;
    start = clock();
    node * array = calloc(size,sizeof(node));

    FILE *inputFile =fopen("input.txt","r");
    if(inputFile==NULL) {
        printf("error opening file");
        return 0;
    }

    char line[100];
    while(fscanf(inputFile, "%49s", line) == 1) {
        insert(line,array);
    }
    topTinFrequentWords(array);
    end = clock();
    double time=(double)(end -start) / CLOCKS_PER_SEC;
    printf("the execution time of the program took %f",time);
    return 0;
}