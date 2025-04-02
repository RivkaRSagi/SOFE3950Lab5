/*
Iqra Zahid - 100824901
Rhea Mathias - 100825543
Rivka Sagi - 100780926
Julian Olano Medina - 100855732
*/

#include <pthread.h> // For threads and mutexes
#include <stdbool.h> // For Bools
#include <stdio.h>   // For I/O
#include <stdlib.h>  // For memory and random numbers
#include <sys/types.h>
#include <time.h>   // For seeding rand()
#include <unistd.h> // For sleep

#define NUMBER_OF_CUSTOMERS 5 // Total number of customer threads
#define NUMBER_OF_RESOURCES 3 // Types of resources in the system

// Global resource arrays
int available[NUMBER_OF_RESOURCES];
int maximum[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
int allocation[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
int need[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];

// Mutex for thread safety
pthread_mutex_t mutex;       // For Request and Release Methods
pthread_mutex_t print_mutex; // New mutex for print protection

// Helper function prototypes
void parse_available_resources_from_arguments(int argc, char *argv[]);
void initialize_banker_matrices(void);
void *customer_thread(void *arg);
void create_customer_threads(void);
int request_resources(int custNum, int request[]);
int release_resources(int custNum, int release[]);
bool is_safe_state(void); // (Banker's Algorithm)

// Parse command-line arguments to initialize available resources
void parse_available_resources_from_arguments(int argc, char *argv[]) {
  if (argc != NUMBER_OF_RESOURCES + 1) {
    fprintf(stderr, "Usage: %s <res1> <res2> <res3>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
    char *end;
    long value = strtol(argv[i + 1], &end, 10);

    if (*end != '\0' || value > INT_MAX ||
        value < 0) { // Detects invalid sequences
      fprintf(stderr, "Invalid resource value: %s\n", argv[i + 1]);
      exit(EXIT_FAILURE);
    }

    available[i] = (int)value;
  }
}

// Set need, maximum, and allocation tables to zero
void initialize_banker_matrices(void) {
  srand(time(NULL)); // seed random value
  for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
    for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
      maximum[i][j] = rand() % (available[j] + 1); // Max <= available
      allocation[i][j] = 0;
      need[i][j] = maximum[i][j]; // Initially, need = maximum
    }
  }
}

int main(int argc, char *argv[]) {

  // Initialize resources
  parse_available_resources_from_arguments(argc, argv);
  initialize_banker_matrices();
  
  // Initialize mutexes
  if (pthread_mutex_init(&mutex, NULL) != 0) {
    fprintf(stderr, "Mutex initialization failed\n");
    exit(EXIT_FAILURE);
  }
  if (pthread_mutex_init(&print_mutex, NULL) != 0) {
    fprintf(stderr, "Print mutex initialization failed\n");
    exit(EXIT_FAILURE);
  }

  pthread_t threads[NUMBER_OF_CUSTOMERS];
  int created_threads = 0; // Track successfully created threads

  // Create customer threads
  for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
    int *custNum = malloc(sizeof(int));
    if (!custNum) {
      fprintf(stderr, "Memory allocation failed for thread %d\n", i);
      goto cleanup;
    }
    *custNum = i;
    if (pthread_create(&threads[i], NULL, customer_thread, custNum) != 0) {
      fprintf(stderr, "Error creating thread %d\n", i);
      free(custNum); // Free memory if thread creation fails
      goto cleanup;
    }
    created_threads++; // Increment only on success
  }

  // Wait for all customer threads
  for (int i = 0; i < created_threads; i++) {
    pthread_join(threads[i], NULL);
  }

cleanup:
  // Destroy mutex and return appropriate status
  if (pthread_mutex_destroy(&mutex) != 0) {
    fprintf(stderr, "Mutex destruction failed\n");
  }
  if (pthread_mutex_destroy(&print_mutex) != 0) {
    fprintf(stderr, "Print mutex destruction failed\n");
  }
  return created_threads == NUMBER_OF_CUSTOMERS
             ? 0
             : 1; // 0 if all threads created, 1 otherwise
}

// Customer thread function
//
// This function simulates the behavior of a customer in a resource allocation
// system. It generates random resource requests, tries to acquire resources,
// and releases them once work is simulated. It also prints the status of
// requests and releases. The function runs in an infinite loop until the
// program is terminated.
void *customer_thread(void *arg) {
  int custNum = *(int *)arg;
  free(arg); // Free the dynamically allocated customer number

  // Initialize a unique seed for this thread to generate random values
  srand((unsigned int)(getpid() + custNum));

  while (true) {
    // Generate random resource request that doesn't exceed the customer's
    // remaining need
    int request[NUMBER_OF_RESOURCES];
    pthread_mutex_lock(&mutex); // Lock while generating request for safety
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
      request[i] = rand() % (need[custNum][i] + 1); // Request <= need
    }
    pthread_mutex_unlock(&mutex); // Unlock after request generation

    // Protect print statements with the print mutex to ensure thread-safe
    // output
    pthread_mutex_lock(&print_mutex);
    printf("Customer %d requests: [%d, %d, %d]\n", custNum, request[0],
           request[1], request[2]);
    pthread_mutex_unlock(&print_mutex);

    // Attempt to request resources
    if (request_resources(custNum, request) == 0) {
      // If resources are successfully allocated, simulate work (e.g., sleep for
      // 1 second)
      sleep(1);

      // Print the resource release status, protected by the print mutex
      pthread_mutex_lock(&print_mutex);
      printf("Customer %d releases: [%d, %d, %d]\n", custNum, request[0],
             request[1], request[2]);
      pthread_mutex_unlock(&print_mutex);

      // Release the resources after the simulated work
      release_resources(custNum, request);
    } else {
      // If the request is denied, print a denial message, protected by the
      // print mutex
      pthread_mutex_lock(&print_mutex);
      printf("Customer %d request denied\n", custNum);
      pthread_mutex_unlock(&print_mutex);
    }

    // Sleep to avoid spamming requests too frequently
    sleep(1);
  }
  return NULL; // Return NULL as required for a pthread function signature
}

// Release resources (atomic)
int release_resources(int custNum, int release[]) {
  // CRITICAL SECTION
  pthread_mutex_lock(&mutex);
  // Check Validity for all resource types
  for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
    if (release[i] > allocation[custNum][i]) {
      pthread_mutex_unlock(&mutex);
      return -1;
    }
  } // If valid, update available, allocation, and need
  for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
    available[i] += release[i];
    allocation[custNum][i] -= release[i];
    need[custNum][i] += release[i];
  }
  pthread_mutex_unlock(&mutex);
  return 0;
}

// Request resources (with safety check)
int request_resources(int custNum, int request[]) {
  // CRITICAL SECTION:
  pthread_mutex_lock(&mutex);
  // Check if request exceeds need, drop it.
  // Also, if insufficient resources are available, drop the request.
  for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
    if (request[i] > need[custNum][i] || (request[i] > available[i])) {
      pthread_mutex_unlock(&mutex);
      return -1; // Request denied
    }
  }
  // Temporarily allocate resources
  for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
    available[i] -= request[i];
    allocation[custNum][i] += request[i];
    need[custNum][i] -= request[i];
  }
  // Check if system is in a safe state (Banker’s Algorithm)
  if (is_safe_state()) {
    pthread_mutex_unlock(&mutex);
    return 0; // Request granted
  } else {
    // Roll back allocation
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
      available[i] += request[i];
      allocation[custNum][i] -= request[i];
      need[custNum][i] += request[i];
    }
    pthread_mutex_unlock(&mutex);
    return -1; // Request denied
  }
}

// Simplified safety algorithm (Banker’s Algorithm)
bool is_safe_state(void) {
  int work[NUMBER_OF_RESOURCES];
  bool finish[NUMBER_OF_CUSTOMERS] = {false};

  // Copy available resources safely
  for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
    work[i] = available[i];
  }

  int count = 0;
  // Find a sequence of customers that can finish
  while (count < NUMBER_OF_CUSTOMERS) {
    bool found = false;
    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
      if (!finish[i]) {
        bool can_finish = true;
        for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
          if (need[i][j] > work[j]) {
            can_finish = false;
            break;
          }
        }
        if (can_finish) {
          // Allocate resources to work
          for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
            work[j] += allocation[i][j];
          }
          finish[i] = true;
          found = true;
          break;
        }
      }
    }
    if (!found)
      return false; // No customer can proceed (no safe sequence)
    count++;
  }
  return true; // Safe state found
}
