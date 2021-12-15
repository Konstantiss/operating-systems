#include <stdio.h> 
#include <unistd.h> 
#include <stdlib.h>
#include <limits.h>

void writeBinary( char *name, int *minmax ){
    FILE *write_ptr = fopen(name,"wb");
    fwrite(minmax,sizeof(int),2,write_ptr);
    fclose(write_ptr);
}

void minmaxAllListsFork(int **numbers, int nList, int *nElem){
    int i;
    int status;
    int n = nList;
    pid_t pids[n];

    /* Start children. */
    for (i = 0; i < n; ++i) {
        if ((pids[i] = fork()) < 0) {
            printf("Fork error\n");
            //abort();
            exit(0);
        } else if (pids[i] == 0) {
            //printf("I'm child %d\n", i);
            int j;
            int min = INT_MIN;
            int max = INT_MAX;
            for (j = 0; j < nElem[i]; j++){
                if(min>numbers[i][j]) min=numbers[i][j];   
		        if(max<numbers[i][j])max=numbers[i][j];    
            }
            char name[12];
            int minmax[2];
            snprintf(name, 12, "minmax-<%d>", i);
            minmax[0] = min;
            minmax[1] = max;
            writeBinary(name, minmax);
            exit(0);
        }
    }

    /* Wait for children to exit. */
    pid_t pid;
    while (n > 0) {
        pid = wait(&status);
        printf("Child with PID %ld exited with status 0x%x.\n", (long)pid, status);
        --n; 
    }    
}

int main(){
    int numberOfRows = 3;
    int numberOfColumns = 5;
    int i,j;
    int** numbers = (int**)malloc(numberOfRows * sizeof(int*));
    for (i = 0; i < numberOfRows; i++)
        numbers[i] = (int*)malloc(numberOfColumns * sizeof(int));
    for (i = 0; i < numberOfRows; i++){
        for (j = 0; j < numberOfColumns; j++){
            numbers[i][j] = j;
            printf("%d", numbers[i][j]);
        }
        printf("\n");
    }
    int nList = 3;
    int nElem[3] = {5, 5, 5};
    minmaxAllListsFork(numbers, nList, nElem);
    return 1;
}