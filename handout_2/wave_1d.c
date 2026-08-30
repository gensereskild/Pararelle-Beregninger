#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>


// Option to change numerical precision.
typedef int64_t int_t;
typedef double real_t;

// Simulation parameters: size, step count, and how often to save the state.
const int_t
    N = 1024,
    max_iteration = 4000,
    snapshot_freq = 20;

// Wave equation parameters, time step is derived from the space step.
//Jeg tror dx er avstanden mellom hvert punkt. (Antar punktene har en sett avstand
//mellom hverandre, men styrken endres).
const real_t
    c  = 1.0,
    dx  = 1.0/N;
real_t
    dt;

// Buffers for three time steps, indexed with 2 ghost points for the boundary.
real_t
    *buffers[3] = { NULL, NULL, NULL };


#define U_prv(i) buffers[0][(i)+1]
#define U(i)     buffers[1][(i)+1]
#define U_nxt(i) buffers[2][(i)+1]


// Save the present time step in a numbered file under 'data/'.
void domain_save ( int_t step )
{
    char filename[256];
    sprintf ( filename, "data/%.5ld.dat", step );
    FILE *out = fopen ( filename, "wb" );
    fwrite ( &U(0), sizeof(real_t), N, out );
    fclose ( out );
}


// TASK: T1
// Set up our three buffers, fill two with an initial cosine wave,
// and set the time step.
void domain_initialize ( void )
{
// BEGIN: T1
    buffers[0] = calloc(N, sizeof(real_t));
    buffers[1] = calloc(N, sizeof(real_t));
    buffers[2] = calloc(N, sizeof(real_t));


    //meningen er jo at det skal starte som en hel cosinus kurve....
    printf("Verdi til dx %f \n", dx);
    printf("Verdi til 200 cosinus %f \n", cos(2*M_PI*200*dx));
    for(int i = 0; i<N; i++){
        U(i) = cos(2*M_PI*i*dx);
    }

    printf("Verdi av start condition %f \n", U(0));

    //Time step er satt til 1.
    dt = dx/c;
    ;
// END: T1
}


// TASK T2:
// Return the memory to the OS.
// BEGIN: T2
void domain_finalize ( void )
{   
    free(buffers[0]);
    free(buffers[1]);
    free(buffers[2]);
    ;
}
// END: T2


// TASK: T3
// Rotate the time step buffers.
// BEGIN: T3
//Flytter et steg fram, forkaster siste.
void domain_shift_forward ( void ){
    free(buffers[0]);
    buffers[0] = buffers[1];
    buffers[1] = buffers[2];
    buffers[2]=calloc(N, sizeof(real_t));
}
    ;
// END: T3


// TASK: T4
// Derive step t+1 from steps t and t-1.
// BEGIN: T4
void calculate_step ( void ){
    for(int i = 0; i<N; i++){
        U_nxt(i) = (-U_prv(i)+2*U(i) + (c*dt/dx) * (U(i-1) + U(i+1) - 2*U(i)));
    }
}
    ;
// END: T4


// TASK: T5
// Neumann (reflective) boundary condition.
// BEGIN: T5
void copy_boundary ( void ){
    U(0) = U(2);
    U(N-1) = U(N-3);

    //U(0) = 0;
    //U(N-1) = 0;
}
    ;
// END: T5


// TASK: T6
// Main time integration.
void simulate( void )
{
// BEGIN: T6
    int_t iteration=0;

    for(; iteration<max_iteration; iteration++){
        copy_boundary();

        calculate_step();

        
        if(iteration%snapshot_freq==0){
            domain_save ( iteration / snapshot_freq );
        }
        domain_shift_forward();
    }
// END: T6
}


int main ( void )
{
    domain_initialize();

    simulate();
    
    domain_finalize();
    exit ( EXIT_SUCCESS );
}
