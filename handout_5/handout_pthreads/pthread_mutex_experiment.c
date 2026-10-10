#include <stdio.h>
#include <pthread.h>

int global_count = 0;

pthread_mutex_t lock;
//Uten locks vil den ikke alltid printe tallene i stigendre rekkefølge.
void* counter(void *arg){
    
    printf("test \n");
    pthread_mutex_lock(&lock);
    for(int i = 0; i<10; i++){
        global_count++;
        printf("verdi til global count %d \n", global_count);
    }
    pthread_mutex_unlock(&lock);
    return NULL;
}

int main(int argc, char **argv){
    pthread_mutex_init(&lock, NULL);
    int number_of_threads = 4;
    pthread_t threads[number_of_threads];
    for(int i = 0; i<number_of_threads; i++){
        pthread_create(&threads[i], NULL, &counter, NULL);
    }
    
    for(int i = 0; i<number_of_threads; i++){
        pthread_join(threads[i], NULL);
    }

    printf("hvorfunker ikke \n");

    return 0;
}