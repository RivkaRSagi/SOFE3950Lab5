#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

//define system resources
#define RESOURCE_A 10
#define RESOURCE_B 5
#define RESOURCE_C 7

//define available system resources (initially)
int available[] = {RESOURCE_A, RESOURCE_B, RESOURCE_C};

//define process struct
typedef struct process{
    int max[3];
    int alloc[3];
    int need[3];
}process;

void printProcess(process p){
    printf("max %d %d %d\nallocated %d %d %d\nneeded %d %d %d\n", 
        p.max[0], p.max[1], p.max[2], p.alloc[0], p.alloc[1], p.alloc[2],
    p.need[0], p.need[1], p.need[2]);
}


void transferArray(int source[3], int (*dest)[3]){
    for(int i=0; i<3; i++){
        (*dest)[i] = source[i];
    }
}

void transfer2Arrays(int source[3], int (*dest)[3], int source2[3], int (*dest2)[3]){
    for(int i=0; i<3; i++){
        (*dest)[i] = source[i];
        (*dest2)[i] = source2[i];
    }
}

int main(){
    //create initial processes
    process P[5];
    int max[5][3] = {{7, 5, 3}, {3,2,2}, {9,0,2}, {2,2,2}, {4,3,3}};
    int alloc[5][3] = {{0,1,0}, {2,0,0}, {3,0,2}, {2,1,1}, {0,0,2}};

    transfer2Arrays(max[0], &P[0].max, alloc[0], &P[0].alloc);
    transfer2Arrays(max[1], &P[1].max, alloc[1], &P[1].alloc);
    transfer2Arrays(max[2], &P[2].max, alloc[2], &P[2].alloc);
    transfer2Arrays(max[3], &P[3].max, alloc[3], &P[3].alloc);
    transfer2Arrays(max[4], &P[4].max, alloc[4], &P[4].alloc);

    //calculate need for each process
    for(int i=0; i<5; i++){
        P[i].need[0] = P[i].max[0]-P[i].alloc[0];
        P[i].need[1] = P[i].max[1]-P[i].alloc[1];
        P[i].need[2] = P[i].max[2]-P[i].alloc[2];
    }

    for(int i=0; i<5; i++){
        printf("process P%d\n", i);
        printProcess(P[i]);
    }


    //Calculate available: total - allocated 
    int tempSum[3] = {0, 0, 0};
    for(int i=0; i<5; i++){//for every process
        for(int j=0; j<3; j++){//for every resource
            tempSum[j] += P[i].alloc[j];
        }
    }

    printf("\ncurrently available in system:\n");
    for(int i=0; i<3; i++){
        available[i] -= tempSum[i];
        printf("%d ", available[i]);
    }
    printf("\n");

    //Bankers algorithm: check need array for a process with resource 
    //  needs that can be fully allocated
    int safeSequence[5];
    int safeIndex = 0;
    int loops = 0;
    bool finish[5] = {false, false, false, false, false};
    bool safeProcess = false;
    bool safe = finish[0] + finish[1] + finish[2] + finish[3] + finish[4];

    do{ 
        //printf("loop #%d\n", loops);
        for(int i=0; i<5; i++){//for each process
            //printf("checking process P%d whos finish status is %d\n", i, finish[i]);
            if(finish[i] == true){continue;};
            for(int j=0;j<3;j++){//for each resource
                if((available[j]-P[i].need[j]) >= 0){//if (available - needed) is positive
                    //printf("available[%d]-process[%d].need[%d] = %d\n", j, i, j, available[j]-P[i].need[j]);
                    safeProcess = true;//assume process is safe
                }else{safeProcess = false; break;}
            }
            if(safeProcess){//if a safe process is found
                //printf("process %d is safe, adding to sequence at %d\n", i, safeIndex);
                safeSequence[safeIndex] = i;
                safeIndex++;
                finish[i] = true;
                for(int j=0; j<3; j++){//assume process is completed and all resources returned
                    available[j] += P[i].alloc[j];
                }
            }else{safeProcess = false;}
        }
        loops++;
        safe = finish[0] && finish[1] && finish[2] && finish[3] && finish[4];
        //printf("safe status: %d, loops: %d\n", safe, loops);
    }while((!safe) && (loops<5));

    if(safe){
        printf("safe sequence: ");
        for(int i=0; i<5; i++){
            printf("%d ", safeSequence[i]);
        }
        printf("\n");
    }
    else{
        printf("no safe sequence was found for these processes\n");
    }





    return 0;
}