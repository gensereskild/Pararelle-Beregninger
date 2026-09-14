#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <sys/time.h>

// TASK: T1a
// Include the MPI headerfile
// BEGIN: T1a
#include <mpi.h>
;
// END: T1a


// Option to change numerical precision.
typedef int64_t int_t;
typedef double real_t;


// TASK: T1b
// Declare variables each MPI process will need
// BEGIN: T1b
int rank, size;

int my_domain_start;
int my_domain_stop;
int domain_size;
int final_domain_size;
;
// END: T1b


// Simulation parameters: size, step count, and how often to save the state.
const int_t
    N = 65536,
    //N = 655360,
    max_iteration = 100000,
    snapshot_freq = 500;

// Wave equation parameters, time step is derived from the space step.
const real_t
    c  = 1.0,
    dx = 1.0;
real_t
    dt;

// Buffers for three time steps, indexed with 2 ghost points for the boundary.
real_t
    *buffers[3] = { NULL, NULL, NULL };


#define U_prv(i) buffers[0][(i)+1]
#define U(i)     buffers[1][(i)+1]
#define U_nxt(i) buffers[2][(i)+1]


// Convert 'struct timeval' into seconds in double prec. floating point
#define WALLTIME(t) ((double)(t).tv_sec + 1e-6 * (double)(t).tv_usec)


// TASK: T8
// Save the present time step in a numbered file under 'data/'.
void domain_save ( int_t step )
{
// BEGIN: T8
    char filename[256];
    sprintf ( filename, "data/%.5ld.dat", step );
    FILE *out = fopen ( filename, "wb" );
    fwrite ( &U(0), sizeof(real_t), N, out );
    fclose ( out );
// END: T8
}


// TASK: T3
// Allocate space for each process' sub-grids
// Set up our three buffers, fill two with an initial cosine wave,
// and set the time step.
void domain_initialize ( void )
{
// BEGIN: T3
//Antar rank 0 er lengst til høyre også rank 1 osv....
    //Den siste domain sizen er større, burde ikke være et stort problem så lenge
    //antall processes ikke er for stor.
    domain_size = N/size;
    final_domain_size = domain_size + N%size;

    my_domain_start = rank * domain_size;
    my_domain_stop = (rank +1) * domain_size;

    if(rank < size-1 && rank > 0){   
        buffers[0] = malloc ( (domain_size+2)*sizeof(real_t) );
        buffers[1] = malloc ( (domain_size+2)*sizeof(real_t) );
        buffers[2] = malloc ( (domain_size+2)*sizeof(real_t) );

        // printf("%d\n", my_domain_stop);
        // printf("%d \n", my_domain_start);
        // printf("%d \n", domain_size);
    }

    //Hvis rank == 0 så alokerer vi plass til hele domenet kalukulerer fortsatt
    //bare sin egen del.
    if(rank == 0){
        buffers[0] = malloc ( (N+2)*sizeof(real_t) );
        buffers[1] = malloc ( (N+2)*sizeof(real_t) );
        buffers[2] = malloc ( (N+2)*sizeof(real_t) );
    }

    if(rank == size-1){
        my_domain_stop = rank*domain_size + final_domain_size;
        domain_size = final_domain_size;

        // printf("%d\n", my_domain_stop);
        // printf("%d \n", my_domain_start);
        // printf("%d \n", domain_size);
        buffers[0] = malloc ( (final_domain_size+2)*sizeof(real_t) );
        buffers[1] = malloc ( (final_domain_size+2)*sizeof(real_t) );
        buffers[2] = malloc ( (final_domain_size+2)*sizeof(real_t) );   
    }

    for ( int_t i=my_domain_start; i<
        my_domain_stop; i++ )
    {
        U_prv(i-my_domain_start) = U(i-my_domain_start) = cos ( 2*M_PI*i / (real_t)N );
    }
// END: T3

    // Set the time step for 1D case.
    dt = dx / c;
}


// Return the memory to the OS.
void domain_finalize ( void )
{
    free ( buffers[0] );
    free ( buffers[1] );
    free ( buffers[2] );
}


// Rotate the time step buffers.
void move_buffer_window ( void )
{
    real_t *temp = buffers[0];
    buffers[0] = buffers[1];
    buffers[1] = buffers[2];
    buffers[2] = temp;
}


// TASK: T4
// Derive step t+1 from steps t and t-1.
void time_step ( void )
{
// BEGIN: T4
    for ( int_t i=0; i<domain_size; i++ )
    {
        U_nxt(i) = -U_prv(i) + 2.0*U(i)
                 + (dt*dt*c*c)/(dx*dx) * (U(i-1)+U(i+1)-2.0*U(i));
    }
// END: T4
}


// TASK: T6
// Neumann (reflective) boundary condition.
void boundary_condition ( void )
{
// BEGIN: T6
if (rank==0) U(-1) = U(1);
if (rank == size-1) U(domain_size) = U(domain_size-2);
// END: T6
}


// TASK: T5
// Communicate the border between processes.
void border_exchange( void )
{
// BEGIN: T5
//Må hente U(-1) basically fra forrige proc og U(domain_size + 1).
    int right_neighbour = rank + 1;
    int left_neighbour = rank - 1;

    int tag1 = 0;
    //hmmmmm, sender til
    //venta hvis jeg tenker, de sender til naboen over seg og motar fra naboen over seg
    if(size != 1){
    if(rank == 0){
        //Denne funksjonen sender og mottar fra høyre nabo.
        MPI_Sendrecv( &(U(domain_size-1)) , 1 , MPI_DOUBLE , right_neighbour , tag1 , &(U(domain_size)) , 1 , MPI_DOUBLE , right_neighbour , tag1 , MPI_COMM_WORLD , MPI_STATUS_IGNORE);
    }
    if(rank > 0 && rank < size-1){
        MPI_Sendrecv( &(U(domain_size-1)) , 1 , MPI_DOUBLE , right_neighbour , tag1 , &(U(domain_size)) , 1 , MPI_DOUBLE , right_neighbour , tag1 , MPI_COMM_WORLD , MPI_STATUS_IGNORE);
        MPI_Sendrecv( &(U(0)) , 1 , MPI_DOUBLE , left_neighbour , tag1 , &(U(-1)) , 1 , MPI_DOUBLE , left_neighbour , tag1 , MPI_COMM_WORLD , MPI_STATUS_IGNORE);
    }
    if(rank == size-1){
        //Denne funksjonen sender og mottar fra venstre nabo
        MPI_Sendrecv( &(U(0)) , 1 , MPI_DOUBLE , left_neighbour , tag1 , &(U(-1)) , 1 , MPI_DOUBLE , left_neighbour , tag1 , MPI_COMM_WORLD , MPI_STATUS_IGNORE);
    }
}
    ;
// END: T5
}


// TASK: T7
// Every process needs to communicate its results
// to root and assemble it in the root buffer
void send_data_to_root()
{
// BEGIN: T7
    if(rank!=0 && rank < size){
        MPI_Send( &U(0) , domain_size , MPI_DOUBLE , 0 , 0 , MPI_COMM_WORLD);
    }
    // if(rank == size-1){
    //     MPI_Send( &U(0) , domain_size , MPI_DOUBLE , 0 , 0 , MPI_COMM_WORLD);
    // }
    //Må accounte for at siste rank har større domain size, fikser etterpå trust.
    MPI_Request requests[size-1];
    //MPI_Request req;

    if(rank == 0){
        int i = 1;
        for(; i<size-1; i++){
            MPI_Irecv( &(U(i*domain_size)) , domain_size , MPI_DOUBLE , i , 0 , MPI_COMM_WORLD , &requests[i-1]);
            // printf("Recieved from %d \n", i);
        }
        //Accounter for den siste som har større domain size
        if(size > 1){
            MPI_Irecv( &(U(i*domain_size)) , final_domain_size , MPI_DOUBLE , size-1 , 0 , MPI_COMM_WORLD , &requests[size-2]);
            MPI_Waitall( size-1 , requests , MPI_STATUS_IGNORE);
        }
// printf("Recieved from %d \n", i);

    }
    ;
// END: T7
}


// Main time integration.
void simulate( void )
{
    // Go through each time step.
    for ( int_t iteration=0; iteration<=max_iteration; iteration++ )
    {
        if ( (iteration % snapshot_freq)==0 )
        {
            send_data_to_root();
            if(rank == 0){
                domain_save ( iteration / snapshot_freq );
            }
        }

        // Derive step t+1 from steps t and t-1.
        border_exchange();
        boundary_condition();
        time_step();

        move_buffer_window();
    }
}


int main ( int argc, char **argv )
{
// TASK: T1c
// Initialise MPI
// BEGIN: T1c
    MPI_Init(&argc, &argv);

    MPI_Comm_rank( MPI_COMM_WORLD , &rank);
    MPI_Comm_size( MPI_COMM_WORLD , &size);

    //printf("my rank is: %d and number of processes is: %d \n", rank, size)
    ;
// END: T1c
    
    struct timeval t_start, t_end;

    domain_initialize();


// TASK: T2
// Time your code
// BEGIN: T2
//Tar tid før og etter simulate
    gettimeofday(&t_start, NULL);
    simulate();
    gettimeofday(&t_end, NULL);
    double time_diff = WALLTIME(t_end) - WALLTIME(t_start);
    printf(" Simulate time %f rank: %d \n", time_diff, rank);
// END: T2
   
    domain_finalize();

// TASK: T1d
// Finalise MPI
// BEGIN: T1d
MPI_Finalize();
    ;
// END: T1d

    exit ( EXIT_SUCCESS );
}
