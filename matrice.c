#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#define MAX_THREADS 4
#define N 5

int B[N][N];
int C[N][N];
int A[N][N];
int T[N];
sem_t mutex;
sem_t empty;
int out = 0;
int totalProduced = 0;
int totalConsumed = 0;

void printMatrix(int matrix[N][N], char *name) {
    printf("Matrix %s:\n", name);
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

void *producer(void *arg) {
    int row = *(int *)arg;
    int result = 0;

    for (int j = 0; j < N; ++j) {
        result += B[row][j] * C[j][row];
    }

    sem_wait(&mutex);
    T[row] = result;
    totalProduced++;
    sem_post(&mutex);
    sem_post(&empty);

    pthread_exit(NULL);
}

void *consumer(void *arg) {
    int col = *(int *)arg;

    while (totalConsumed < N) {
        sem_wait(&empty);
        sem_wait(&mutex);

        int y = out;
        int item = T[y];
        printf("Consumed: %d\n", item);

        // Update the corresponding element in the row
        A[y][col] = item;

        out = (out + 1) % N;
        totalConsumed++;
        sem_post(&mutex);

        sem_post(&empty);
    }

    pthread_exit(NULL);
}

int main() {
    srand((unsigned int)time(NULL));

    // Initialization of matrices B and C with random values
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            B[i][j] = rand() % 10;  // Random values between 0 and 9
            C[i][j] = rand() % 10;
        }
    }

    // Initialization of semaphores
    sem_init(&mutex, 0, 1);
    sem_init(&empty, 0, N);

    // Display matrices B and C
    printMatrix(B, "B");
    printMatrix(C, "C");

    // Producer threads
    pthread_t producerThreads[N];
    for (int i = 0; i < N; ++i) {
        int *arg = malloc(sizeof(*arg));
        *arg = i;
        pthread_create(&producerThreads[i], NULL, producer, arg);
    }

    // Consumer threads
    pthread_t consumerThreads[N];
    for (int i = 0; i < N; ++i) {
        int *arg = malloc(sizeof(*arg));
        *arg = i;
        pthread_create(&consumerThreads[i], NULL, consumer, arg);
    }

    // Wait for the producer threads to finish
    for (int i = 0; i < N; ++i) {
        pthread_join(producerThreads[i], NULL);
    }

    // Wait for the consumer threads to finish
    for (int i = 0; i < N; ++i) {
        pthread_join(consumerThreads[i], NULL);
    }

    // Display the resulting matrix A
    printMatrix(A, "A");

    // Destroy semaphores
    sem_destroy(&mutex);
    sem_destroy(&empty);

    return 0;
}
