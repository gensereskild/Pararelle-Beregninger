#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <errno.h>
#include <sys/time.h>

// TASK: T1a
// Include the pthreads library
// BEGIN: T1a
#include <pthread.h>
;
// END: T1a

// Option to change numerical precision
typedef int64_t int_t;
typedef double real_t;


// TASK: T1b
// Pthread management
// BEGIN: T1b
int_t n_threads = 1;

pthread_barrier_t simulate_barrier;
// END: T1b

// Performance measurement
struct timeval t_start, t_end;
#define WALLTIME(t) ((double)(t).tv_sec + 1e-6 * (double)(t).tv_usec) 

// Simulation parameters: size, step count, and how often to save the state
const int_t
    N = 1024,
    //endrer til 100 max iterations
    //var 4000 til vanlig
    //men ser at sequential er 2000, og snapshot frequency 20
    max_iteration = 2000,
    snapshot_freq = 20;

// Wave equation parameters, time step is derived from the space step
const real_t
    c  = 1.0,
    h  = 1.0;
real_t
    dt;

// Buffers for three time steps, indexed with 2 ghost points for the boundary
real_t
    *buffers[3] = { NULL, NULL, NULL };

#define U_prv(i,j) buffers[0][((i)+1)*(N+2)+(j)+1]
#define U(i,j)     buffers[1][((i)+1)*(N+2)+(j)+1]
#define U_nxt(i,j) buffers[2][((i)+1)*(N+2)+(j)+1]


// Rotate the time step buffers.
void move_buffer_window ( void )
{
    real_t *temp = buffers[0];
    buffers[0] = buffers[1];
    buffers[1] = buffers[2];
    buffers[2] = temp;
}


// Set up our three buffers, and fill two with an initial perturbation
void domain_initialize ( void )
{
    buffers[0] = malloc ( (N+2)*(N+2)*sizeof(real_t) );
    buffers[1] = malloc ( (N+2)*(N+2)*sizeof(real_t) );
    buffers[2] = malloc ( (N+2)*(N+2)*sizeof(real_t) );

    for ( int_t i=0; i<N; i++ )
    {
        for ( int_t j=0; j<N; j++ )
        {
            real_t delta = sqrt ( ((i-N/2)*(i-N/2)+(j-N/2)*(j-N/2))/(real_t)N );
            U_prv(i,j) = U(i,j) = exp ( -4.0*delta*delta );
        }
    }

    // Set the time step
    dt = (h*h) / (4.0*c*c);
}


// Get rid of all the memory allocations
void domain_finalize ( void )
{
    free ( buffers[0] );
    free ( buffers[1] );
    free ( buffers[2] );
}


// TASK: T3
// Integration formula
void time_step ( int_t thread_id )
{
// BEGIN: T3
//utifra oppgaven virker det som om de vil ha en round robin calculation style.
//kan endre enten i eller j, vet lowkey ikke hva som er best for performance.
    for ( int_t i=0; i<N; i+=1 )
        for ( int_t j=thread_id; j<N; j+= n_threads )
            U_nxt(i,j) = -U_prv(i,j) + 2.0*U(i,j)
                     + (dt*dt*c*c)/(h*h) * (
                        U(i-1,j)+U(i+1,j)+U(i,j-1)+U(i,j+1)-4.0*U(i,j)
                     );
// END: T3
}


// TASK: T4
// Neumann (reflective) boundary condition
void boundary_condition ( int_t thread_id )
{
// BEGIN: T4
//Gjør det samme her ved å endre i og j
    for ( int_t i=thread_id; i<N; i+=n_threads )
    {
        U(i,-1) = U(i,1);
        U(i,N)  = U(i,N-2);
    }
    for ( int_t j=thread_id; j<N; j+=n_threads )
    {
        U(-1,j) = U(1,j);
        U(N,j)  = U(N-2,j);
    }
// END: T4
}


// Save the present time step in a numbered file under 'data/'
void domain_save ( int_t step )
{
    char filename[256];
    sprintf ( filename, "data/%.5ld.dat", step );
    FILE *out = fopen ( filename, "wb" );
    for ( int_t i=0; i<N; i++ )
        fwrite ( &U(i,0), sizeof(real_t), N, out );
    fclose ( out );
}


// TASK: T5
// Main loop
void *simulate ( void *id )
{
// BEGIN: T5
    // Go through each time step
    int t_id = (int)(intptr_t) id;
    printf("My thread id is %d, of total threads %ld \n", t_id, n_threads);

    for ( int_t iteration=0; iteration<=max_iteration; iteration++ )
    {
        //La til slik at bare thread id0 lagerer ved snapshots.
        if ( ((iteration % snapshot_freq)==0) && (t_id == 0))
        {
            domain_save ( iteration / snapshot_freq );
        }
        pthread_barrier_wait(&simulate_barrier);
        // Derive step t+1 from steps t and t-1
        boundary_condition(t_id);
        pthread_barrier_wait(&simulate_barrier);
        time_step(t_id);

        // Rotate the time step buffers
        pthread_barrier_wait(&simulate_barrier);
        //Burde ikke bare 1 prossess gjøre det her...
        if(t_id == 0){
            move_buffer_window();
        }
        pthread_barrier_wait(&simulate_barrier);
    }
// END: T5
}


// Main time integration loop
int main ( int argc, char **argv )
{
    // Number of threads is an optional argument, sanity check its value
    if ( argc > 1 )
    {
        n_threads = strtol ( argv[1], NULL, 10 );
        if ( errno == EINVAL )
            fprintf ( stderr, "'%s' is not a valid thread count\n", argv[1] );
        if ( n_threads < 1 )
        {
            fprintf ( stderr, "Number of threads must be >0\n" );
            exit ( EXIT_FAILURE );
        }
    }
    
    // TASK: T1c
    // Initialise pthreads
    // BEGIN: T1c
    pthread_barrier_init(&simulate_barrier, NULL, n_threads);
    
    pthread_t threads[n_threads];
    
    ;
    // END: T1c
    
    // Set up the initial state of sithe domain
    domain_initialize();
    
    // Time the execution
    gettimeofday ( &t_start, NULL );
    
    // TASK: T2
// Run the integration loop
// BEGIN: T2
for(int i = 0; i<n_threads; i++){
    pthread_create(&threads[i], NULL, &simulate, (void*)(intptr_t)i);
}

for(int i = 0; i<n_threads; i++){
    pthread_join(threads[i], NULL);
}
printf("Blir vi noengang ferdig med simulate?\n");
//simulate(NULL);
// END: T2

    // Report how long we spent in the integration stage
    gettimeofday ( &t_end , NULL );
    printf ( "%lf seconds elapsed with %ld threads\n",
        WALLTIME(t_end)-WALLTIME(t_start),
        n_threads
    );

    // Clean up and shut down
    domain_finalize();

// TASK: T1d
// Finalise pthreads
// BEGIN: T1d


pthread_barrier_destroy(&simulate_barrier);
    ;
// END: T1d

    exit ( EXIT_SUCCESS );
}
