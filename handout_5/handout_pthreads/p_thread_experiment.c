#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

int number_of_threads;

void* print_hello(void *arg){
    printf("Print NOEEEEE \n");
    printf("Hello world from %lu \n", pthread_self());
    int tid = (int) arg;
    printf("Thread id %d \n", tid);
    return 14;
}

int main(int argc, char **argv){
    printf("Verdi til argv %s \n", argv[0]);

    number_of_threads = atoi(argv[1]);

    printf("Number of threads activated %d \n", number_of_threads);

    pthread_t threads[number_of_threads];

    for(int i = 0; i<number_of_threads; i++){
        pthread_create(&threads[i], NULL, &print_hello, (void *)i);
    }
    
    int my_return_data;
    for(int i = 0; i<number_of_threads; i++){
        pthread_join(threads[i], &my_return_data);
    }

    printf("value of my return data %d \n", my_return_data);

    return 0;
}