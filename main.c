#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>


#define NUM_THREADS 4
#define BUFFER_SIZE 10

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    char data[BUFFER_SIZE][1000];
    int head;
    int tail;
    int count;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} Buffer;

void *dummy_consumer(void *arg) {
    Buffer *buffer = (Buffer *)arg;
    char line[1000];

    while (1) {
        pthread_mutex_lock(&buffer->mutex);

        // Wait if the buffer is empty
        while (buffer->count == 0) {
            pthread_cond_wait(&buffer->not_empty, &buffer->mutex);
        }

        // Read the next line from the buffer
        strcpy(line, buffer->data[buffer->tail]);
        buffer->tail = (buffer->tail + 1) % BUFFER_SIZE;
        buffer->count--;

        // Signal the Input Thread that there's space
        pthread_cond_signal(&buffer->not_full);
        pthread_mutex_unlock(&buffer->mutex);

        // Check for the end marker
        if (strcmp(line, "STOP\n") == 0) {
            break;
        }

        // Print the line to verify correctness
        printf("Consumed: %s", line);
    }

    return NULL;
}


void buffer_init(Buffer *buffer) {
    buffer->head = 0;
    buffer->tail = 0;
    buffer->count = 0;
    pthread_mutex_init(&buffer->mutex, NULL);
    pthread_cond_init(&buffer->not_empty, NULL);
    pthread_cond_init(&buffer->not_full, NULL);
}

void *input_thread(void *arg){
    Buffer *buffer = (Buffer *)arg;
    char line[1000];
    

    while(fgets(line, sizeof(line), stdin) != NULL){

        if(strcmp(line, "STOP\n") == 0){
            break;
        }


        pthread_mutex_lock(&buffer->mutex);

        //wait if the buffer is full
        while(buffer->count == BUFFER_SIZE){
            pthread_cond_wait(&buffer->not_full, &buffer->mutex);
        }

        //copy the line into buffer
        strcpy(buffer->data[buffer->head], line);
        buffer->head = (buffer->head + 1) % BUFFER_SIZE;
        buffer->count++;

        pthread_cond_signal(&buffer->not_empty);
        pthread_mutex_unlock(&buffer->mutex);
    }

    pthread_mutex_lock(&buffer->mutex);
    while(buffer->count == BUFFER_SIZE){
        pthread_cond_wait(&buffer->not_full, &buffer->mutex);
    }
    strcpy(buffer->data[buffer->head], "STOP");
    buffer->head = (buffer->head + 1) % BUFFER_SIZE;
    buffer->count++;
    pthread_cond_signal(&buffer->not_empty);
    pthread_mutex_unlock(&buffer->mutex);

    return NULL;
}

void *plus_sign_thread(void *arg){
    Buffer *buffer = (Buffer *)arg;
}



int main(){
    Buffer input_buffer;
    buffer_init(&input_buffer);

    pthread_t input_tid, consumer_tid;

    pthread_create(&input_tid, NULL, input_thread, &input_buffer);
   // pthread_create(&consumer_tid, NULL, dummy_consumer, &input_buffer);



    pthread_join(input_tid, NULL);
    //pthread_join(consumer_tid, NULL);

    return 0;
}