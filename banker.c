/*
Iqra Zahid - 100824901
Rhea Mathias - 100825543
Rivka Sagi - 100780926 
Julian Olano Medina - 100855732 
*/

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define NUMBER_OF_CUSTOMERS 5
#define NUMBER_OF_RESOURCES 3

//define system resources
#define RESOURCE_A 10
#define RESOURCE_B 5
#define RESOURCE_C 7

//define available system resources (initially)
int available[NUMBER_OF_RESOURCES] = {RESOURCE_A, RESOURCE_B, RESOURCE_C};

//max demand of each customer
int maximum[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES] =
        {{7, 5, 3}, {3,2,2}, {9,0,2}, {2,2,2}, {4,3,3}};

//amount currently allocated to each customer
int allocation[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES] = 
        {{0,1,0}, {2,0,0}, {3,0,2}, {2,1,1}, {0,0,2}};

//the remaining need of each customer
int need[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];

//last recorded safe sequence
int safeSequence[5];

void transferArray(int source[3], int dest[3]){
    for(int i=0; i<3; i++){
        dest[i] = source[i];
    }
}

//requestResources() uses the bankers algorithm to check if the request for resources is safe
//  returns 0 if successful and -1 if not
int requestResources(){
    int availableTemp[NUMBER_OF_RESOURCES];
    transferArray(available, availableTemp);
    int safeSeq[NUMBER_OF_CUSTOMERS];
    int safeIndex = 0;
    int loops = 0;
    bool finish[NUMBER_OF_CUSTOMERS] = {false, false, false, false, false};
    bool safeCustomer = false;
    bool safe = false;

    do{
        for(int i=0; i<NUMBER_OF_CUSTOMERS; i++){//for each customer 
            if(finish[i] == true){continue;}; 

            for(int j=0; j<NUMBER_OF_RESOURCES; j++){//for each resource
                if((available[j]-need[i][j]) >= 0){//if (available - needed) is positive
                    safeCustomer = true; //assume customer is safe
                }else{safeCustomer = false; break;} //customer is not safe
            }
            if(safeCustomer){// if safe customer is found, add to sequence
                safeSeq[safeIndex] = i;
                safeIndex++;
                finish[i] = true;
                for(int j=0; j<NUMBER_OF_RESOURCES; j++){
                    availableTemp[j] += allocation[i][j];
                }
            }
        }
        loops++;
        safe = finish[0] && finish[1] && finish[2] && finish[3] && finish[4];
    }while((!safe) && (loops<5));

    if(safe){//succussful (request is safe)
        printf("current safe sequence for system: ");
        for(int i=0; i<NUMBER_OF_CUSTOMERS; i++){
            printf("%d ", safeSeq[i]);
        }
        printf("\n");
        transferArray(safeSeq, safeSequence);//record the current safe sequence globally
        return 0;
    }
    else{//unsuccessful (request is unsafe)
        printf("request puts system in unsafe state, rejecting request.\n");
        int zero[NUMBER_OF_RESOURCES] = {0,0,0};
        return -1;
    }
}

//releaseResources() allocates requested resource to customer, waits until customer finishes,
//  then releases the resources back to the system
int releaseResources(int custNum){
    //return allocated resources to available
    for(int i=0; i<NUMBER_OF_RESOURCES; i++){
        available[i] += allocation[custNum][i];
    }
    //free the space that the customer request was using
    int zero[3] = {0,0,0};
    transferArray(zero, maximum[custNum]);
    transferArray(zero, allocation[custNum]);
    transferArray(zero, need[custNum]);

    return 0;

}

int main(){
    //initialize global processes

    //calculate initial need for each process
    for(int i=0; i<NUMBER_OF_CUSTOMERS; i++){
        for(int j=0; j<NUMBER_OF_RESOURCES; j++){
            need[i][j] = maximum[i][j]-allocation[i][j];
        }
    }

    //Calculate initial available: total - allocated 
    int tempSum[3] = {0, 0, 0};
    for(int i=0; i<5; i++){//for every process
        for(int j=0; j<3; j++){//for every resource
            tempSum[j] += allocation[i][j];
        }
    }

    printf("\ncurrently available in system:\n");
    for(int i=0; i<3; i++){
        available[i] -= tempSum[i];
        printf("%d ", available[i]);
    }
    printf("\n");

    
    //run bankers algorithm on initial global variables
    if(requestResources() == 0){//if initial bankers algorithm works
        //(create a thread?) and run the first in the sequence
    }else{//if bankers algorithm fails on initial set up
        //empty global array and wait for requests
    }

    bool keepLooping = true;
    while(keepLooping){
        //create thread
        //ask customer for input

        //ask for input
        //


        //ask for customer input


    }
        //ask for customer input
        //replace process global variables
        //run bankers algorithm
            //if safe, run the first in sequence
            //if unsafe, reset process global variable to 0




    return 0;
}
