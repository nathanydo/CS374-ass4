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

        // Check for the stop marker
        if (consume_input_buffer(buffer, line)) {
            break;
        }

        printf("Processed Line: %s\n", line);
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


// void *input_thread(void *arg){
//     Buffer *buffer = (Buffer *)arg;
//     char line[1000];
    

//     while(fgets(line, sizeof(line), stdin) != NULL){

//         if(strcmp(line, "STOP\n") == 0){
//             break;
//         }


//         pthread_mutex_lock(&buffer->mutex);

//         //wait if the buffer is full
//         while(buffer->count == BUFFER_SIZE){
//             pthread_cond_wait(&buffer->not_full, &buffer->mutex);
//         }

//         //copy the line into buffer
//         strcpy(buffer->data[buffer->head], line);
//         buffer->head = (buffer->head + 1) % BUFFER_SIZE;
//         buffer->count++;

//         pthread_cond_signal(&buffer->not_empty);
//         pthread_mutex_unlock(&buffer->mutex);
//     }

//     pthread_mutex_lock(&buffer->mutex);
//     while(buffer->count == BUFFER_SIZE){
//         pthread_cond_wait(&buffer->not_full, &buffer->mutex);
//     }
//     strcpy(buffer->data[buffer->head], "STOP");
//     buffer->head = (buffer->head + 1) % BUFFER_SIZE;
//     buffer->count++;
//     pthread_cond_signal(&buffer->not_empty);
//     pthread_mutex_unlock(&buffer->mutex);

//     return NULL;
// }

int consume_input_buffer(Buffer *buffer, char *line){
    pthread_mutex_lock(&buffer->mutex);
    while (buffer->count == 0) {
        pthread_cond_wait(&buffer->not_empty, &buffer->mutex);
    }
    strcpy(line, buffer->data[buffer->tail]);
    buffer->tail = (buffer->tail + 1) % BUFFER_SIZE;
    buffer->count--;
    pthread_cond_signal(&buffer->not_full);
    pthread_mutex_unlock(&buffer->mutex);

    // Return 1 if the line is "STOP", otherwise 0
    return strcmp(line, "STOP") == 0;
}

void produce_output_buffer(Buffer *buffer, const char *line){
    pthread_mutex_lock(&buffer->mutex);
    while (buffer->count == BUFFER_SIZE) {
        pthread_cond_wait(&buffer->not_full, &buffer->mutex);
    }
    strcpy(buffer->data[buffer->head], line);
    buffer->head = (buffer->head + 1) % BUFFER_SIZE;
    buffer->count++;
    pthread_cond_signal(&buffer->not_empty);
    pthread_mutex_unlock(&buffer->mutex);
}

void *input_thread(void *arg) {
    Buffer *buffer = (Buffer *)arg;
    char line[1000];

    while (1) {
        printf("Enter a line (type STOP to finish): ");
        fgets(line, sizeof(line), stdin);

        // Remove trailing newline if present
        line[strcspn(line, "\n")] = '\0';

        produce_line(buffer, line);

        if (strcmp(line, "STOP") == 0) {
            break;
        }
    }

    return NULL;
}



void *line_seperator_thread(void *arg){
    Buffer *input_buffer = ((Buffer **)arg)[0];
    Buffer *output_buffer = ((Buffer **)arg)[1];
    char line[1000];

    while(1){

        if(consume_input_buffer(input_buffer, line)){

            produce_output_buffer(output_buffer, "STOP");
            breadk;
        }

        for(int i = 0; line[i] != '\0'; i++){
            if(line[i] == '\n'){
                line[i] = ' ';
            }
        }

        produce_output_buffer(output_buffer, line);
    }

    return NULL;
}


void *plus_sign_thread(void *arg){
    Buffer *input_buffer = ((Buffer **)arg)[0];
    Buffer *output_buffer = ((Buffer **)arg)[1];
    char line[1000];
    char processed_line[1000];
    int j = 0;

    while(1){
       if(consume_input_buffer(input_buffer, line)){

            produce_output_buffer(output_buffer, "STOP");
            breadk;
        }

        for(int i = 0; line[i] != '\0'l; i++){
            if(line[i] == '+' && line[i + 1] == '+'){
                processed_line[j++] = '^';
                i++;
            }
            else{
                processed_line[j++] = line[i];
            }
        }
        processed_line[j] = '\0';

        produce_output_buffer(output_buffer, processed_line);
    }

    return NULL;
}



int main(){
    Buffer input_buffer;
    Buffer output_buffer;

    buffer_init(&input_buffer);
    buffer_init(&output_buffer);


    pthread_t input_tid, line_separator_tid, plus_sign_tid, consumer_tid;

    Buffer *args[2] = {&input_buffer, &output_buffer};

    pthread_create(&input_tid, NULL, input_thread, &input_buffer);
    pthread_create(&line_separator_tid, NULL, line_seperator_thread, args);
    pthread_create(&plus_sign_tid, NULL, plus_sign_thread, args);
    pthread_create(&consumer_tid, NULL, dummy_consumer, &output_buffer);



    pthread_join(input_tid, NULL);
    pthread_join(line_separator_tid, NULL);
    pthread_join(plus_sign_tid, NULL);
    pthread_join(consumer_tid, NULL);

    return 0;
}