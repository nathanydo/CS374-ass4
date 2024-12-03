#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>


#define BUFFER_SIZE 50
#define INPUT_LENGTH 1000
#define OUTPUT_LENGTH 80


//initialize buffer 1
char buffer_1[BUFFER_SIZE][INPUT_LENGTH];
int count_1 = 0;
int prod_idx_1 = 0;
int cond_idx_1 = 0;
pthread_mutex_t mutex_1 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t full_1 = PTHREAD_COND_INITIALIZER;


//initialize buffer 2
char buffer_2[BUFFER_SIZE][INPUT_LENGTH];
int count_2 = 0;
int prod_idx_2 = 0;
int cond_idx_2 = 0;
pthread_mutex_t mutex_2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t full_2 = PTHREAD_COND_INITIALIZER;


//intialize buffer 3
char buffer_3[BUFFER_SIZE][INPUT_LENGTH];
int count_3 = 0;
int prod_idx_3 = 0;
int cond_idx_3 = 0;
pthread_mutex_t mutex_3 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t full_3 = PTHREAD_COND_INITIALIZER;


/**
 * Same for the rest of put_buffers.
 */
void put_buff_1(char *line){ 
    pthread_mutex_lock(&mutex_1); // Lock the mutex to ensure thread safety
    strcpy(buffer_1[prod_idx_1], line);
    prod_idx_1 = (prod_idx_1 + 1) % BUFFER_SIZE; // Update production index, wrapping around at BUFFER_SIZE
    count_1++;  // Increment count of items in buffer_1
    pthread_cond_signal(&full_1); // Signal that data is available in the buffer
    pthread_mutex_unlock(&mutex_1); // Unlock the mutex
}



/******************************
 * THREAD 1:
 * Gets the input from the user or from text file
 * Reads and puts it into buffer
 ******************************/
void *input_thread(void *arg) {
    char line[INPUT_LENGTH];


    while(fgets(line, sizeof(line), stdin)){
        if (strncmp(line, "STOP\n", 5) == 0){
            break;
        }
        put_buff_1(line);
    }
    put_buff_1("STOP\n");

    return NULL;
} 



/*************************************** 
 * THREAD 2:
 * Gets the data from buffer 1, while putting buffer 2 into use
 * 
 * Replaces the new lines in the input with spaces
*****************************************/


/**
 * Same for the rest of get_buffers.
 */
char *get_buff_1(){
    pthread_mutex_lock(&mutex_1); // Lock mutex to ensure thread safety
    while(count_1 == 0){ // If buffer_1 is empty, wait for data to be available
        pthread_cond_wait(&full_1, &mutex_1);
    }

    char *line = buffer_1[cond_idx_1]; // Get the line from buffer_1 at the current condition index
    cond_idx_1 = (cond_idx_1 + 1) % BUFFER_SIZE; // Update the condition index, wrapping around at BUFFER_SIZE
    count_1--; // Decrement the count of items in buffer_1
    pthread_mutex_unlock(&mutex_1); // Unlock the mutex
    return line;
}


void put_buff_2(char *line){
    pthread_mutex_lock(&mutex_2);
    strcpy(buffer_2[prod_idx_2], line);
    prod_idx_2 = (prod_idx_2 + 1) % BUFFER_SIZE;
    count_2++;
    pthread_cond_signal(&full_2);
    pthread_mutex_unlock(&mutex_2);
}


void *line_seperator_thread(void *arg) {

    while (1) {
        char *line = get_buff_1();
        if(strncmp(line, "STOP\n", 5) == 0){
            put_buff_2("STOP\n");
            break;
        }
        // Replace newlines with spaces
        for (int i = 0; line[i]; i++) {
            if (line[i] == '\n') {
                line[i] = ' ';
            }
        }
        put_buff_2(line);
    }
    return NULL;
}




/**************************************
 * THREAD 3:
 * Uses data in buffer 2, while placing buffer 3 into use
 * 
 * Replaces pairs of plus, "++", with cheverons, "^".
 ***************************************/
char *get_buff_2(){
    pthread_mutex_lock(&mutex_2);
    while(count_2 == 0){ // If buffer_1 is empty, wait for data to be available
        pthread_cond_wait(&full_2, &mutex_2);
    }

    char *line = buffer_2[cond_idx_2];
    cond_idx_2 = (cond_idx_2 + 1) % BUFFER_SIZE;
    count_2--;
    pthread_mutex_unlock(&mutex_2);
    return line;
}


void put_buff_3(char *line){
    pthread_mutex_lock(&mutex_3);
    strcpy(buffer_3[prod_idx_3], line);
    prod_idx_3 = (prod_idx_3 + 1) % BUFFER_SIZE;
    count_3++;
    pthread_cond_signal(&full_3);
    pthread_mutex_unlock(&mutex_3);
}


void *plus_sign_thread(void *arg) {
    while (1) {
        char *line = get_buff_2();
        if(strncmp(line, "STOP\n", 5) == 0){
            put_buff_3("STOP\n");
            break;
        }
        char processed_line[INPUT_LENGTH];
        int j = 0;

        // Replace newlines with spaces in the line
        for (int i = 0; line[i] != '\0'; i++) {
            if (line[i] == '+' && line[i + 1] == '+') {
                processed_line[j++] = '^';  // Replace with '^'
                i++;  // Skip the next '+'
            } else {
                processed_line[j++] = line[i];
            }
        }
        processed_line[j] = '\0';
        put_buff_3(processed_line);
    }

    return NULL;
}



/**************************************
 * THREAD 4:
 * Uses data in buffer 3
 * 
 * Outputs the processed data in lines of 80 characters.
****************/
char *get_buff_3(){
    pthread_mutex_lock(&mutex_3);
    while(count_3 == 0){
        pthread_cond_wait(&full_3, &mutex_3);
    }

    char *line = buffer_3[cond_idx_3];
    cond_idx_3 = (cond_idx_3 + 1) % BUFFER_SIZE;
    count_3--;
    pthread_mutex_unlock(&mutex_3);
    return line;
}


void *output_thread(void *args) {
    char output_buffer[INPUT_LENGTH * BUFFER_SIZE] = ""; // Buffer to accumulate processed output
    int output_index = 0;
    while (1) {
        char *line = get_buff_3();

        // Stop signal for output
        if (strncmp(line, "STOP\n", 5) == 0) {
            break;
        }
        strcat(output_buffer, line); // Append the line to the output buffer
        output_index += strlen(line);
        while (output_index >= OUTPUT_LENGTH) {
            char output_line[OUTPUT_LENGTH + 1];
            strncpy(output_line, output_buffer, OUTPUT_LENGTH);
            output_line[OUTPUT_LENGTH] = '\0';

            // Print formatted line
            printf("%s\n", output_line);
            memmove(output_buffer, output_buffer + OUTPUT_LENGTH, output_index - OUTPUT_LENGTH + 1);
            output_index -= OUTPUT_LENGTH;
        }
    }
    return NULL;
}





int main() {

    // Create threads
    pthread_t input_tid, line_sep_tid, plus_sign_tid, output_tid;

    pthread_create(&input_tid, NULL, input_thread, NULL);
    pthread_create(&line_sep_tid, NULL, line_seperator_thread, NULL);
    pthread_create(&plus_sign_tid, NULL, plus_sign_thread, NULL);
    pthread_create(&output_tid, NULL, output_thread, NULL);

    // Wait for threads to finish
    pthread_join(input_tid, NULL);
    pthread_join(line_sep_tid, NULL);
    pthread_join(plus_sign_tid, NULL);
    pthread_join(output_tid, NULL);

    return 0;
}