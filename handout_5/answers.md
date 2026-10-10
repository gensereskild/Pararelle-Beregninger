### 1. Why is there no need for a border exchange when using Pthreads?

There is no need for border exchange because the threads share memory, so every thread has access to every datapoint.

### 2. What is the difference between OpenMP and MPI?
MPI is a library that creates is able to setup an environment with multiple processes running, each with their own adress space etc. MPI then supports functionality for sending data across the processes.
OpenMP on the other hand is a library that makes it easy to use posix threads. They differ in that threads share all the memory, except their own local variables on their own stack. OpenMP provides pragma that the compiler reads and adds the necessary code to implement the logic. OpenMP is supported by most compilers, however with MPI you have to compile it with the MPI compiler.

### 3. Inspect the wave 2d barrier.c file. Comment on the difference between Pthreads and the two OpenMP implementations.

So the Pthreads one and the omp barrier one should work pretty much identical, where you create some threads they compute, and they sync up in between computation steps. The difference is that the omp barrier creates an amount of threads that is decided by omp itself, while in the Pthreads you can specify the amount of threads you want. In the workshare openMP one we are only parallelizing the main time step and not the boundary condition. We also creates new threads every single time we do a time step instead of just keeping the same threads, so that may introduce some overhead. It also syncs at the end of the time step computation.

### 4. How would you parallelise a recursion problem with OpenMP?

Create an example to help me reason about this:
rec_function_count_to_100(int counter){
    if(counter<100){
        counter++
        counter = rec_function_count_to_100(counter);
    }
    return counter
}
To parallelize it you would write #omp task before the function call so that any idle threads can pick up the task. Howver this doens't really help you speed up the computation if every result still depends on the previous results. However you could rewrite the recursive function to something like this.

nt rec_function_count_to_100(int counter){
    if(counter<10000){
        counter++;
        for(int i = 0; i<10; i++){
            #pragma omp task
            counter = rec_function_count_to_100(counter);
        }
    }
    return counter;
}
However i think this is really bad because it creates a lot of stack frames. so you would need to make it tail end recursive i would think.

Anyway to answer the original question use #pragma omp task so that any thread can pick it up.