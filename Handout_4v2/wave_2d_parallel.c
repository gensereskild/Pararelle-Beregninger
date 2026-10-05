#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <errno.h>
#include <inttypes.h>

#include "argument_utils.h"

// TASK: T1a
// Include the MPI headerfile
// BEGIN: T1a
#include <mpi.h>
;
// END: T1a


// Convert 'struct timeval' into seconds in double prec. floating point
#define WALLTIME(t) ((double)(t).tv_sec + 1e-6 * (double)(t).tv_usec)

// Option to change numerical precision
typedef int64_t int_t;
typedef double real_t;


// Buffers for three time steps, indexed with 2 ghost points for the boundary
real_t
    *buffers[3] = { NULL, NULL, NULL };

// TASK: T1b
// Declare variables each MPI process will need
// BEGIN: T1b
int world_size;
int world_rank;
int cartesian_rank;
MPI_Comm cartesian_comm;
int dimensions[2];
int my_cords[2];
int number_of_rows;
int number_of_columns;
int index_row_start;
int index_column_start;

//Husk jeg endret fra N til x_span
#define U_prv(i,j) buffers[0][((i)+1)*(number_of_columns+2)+(j)+1]
#define U(i,j)     buffers[1][((i)+1)*(number_of_columns+2)+(j)+1]
#define U_nxt(i,j) buffers[2][((i)+1)*(number_of_columns+2)+(j)+1]
// END: T1b

// Simulation parameters: size, step count, and how often to save the state
int_t
    M = 256,    // rows
    N = 256,    // cols
    max_iteration = 1000,
    snapshot_freq = 10;

// Wave equation parameters, time step is derived from the space step
const real_t
    c  = 1.0,
    dx = 1.0,
    dy = 1.0;
real_t
    dt;




// Rotate the time step buffers.
void move_buffer_window ( void )
{
    real_t *temp = buffers[0];
    buffers[0] = buffers[1];
    buffers[1] = buffers[2];
    buffers[2] = temp;
}


// TASK: T8
// Save the present time step in a numbered file under 'data/'
void domain_save ( int_t step )
{
// BEGIN: T8
    char filename[256];

    // Ensure output directory exists (ignore error if it already exists)
    if (mkdir("data", 0755) != 0 && errno != EEXIST) {
        perror("mkdir data");
        exit(EXIT_FAILURE);
    }

    snprintf(filename, sizeof(filename), "data/%05" PRId64 ".dat", step);
    
    MPI_File file_handle;

    MPI_File_open( cartesian_comm , filename , MPI_MODE_RDWR | MPI_MODE_CREATE ,
         MPI_INFO_NULL , &file_handle);
        
    MPI_File_set_size(file_handle, 0);
    
    //Vi skriver en og en rad størrelsen på rad er number_of_columns fordi det gir mening
    for(int i = 0; i<number_of_rows; i++){
        MPI_File_write_at_all( file_handle , ((index_row_start+i)*N + index_column_start) * sizeof(real_t) ,
         &U(i,0) , number_of_columns , MPI_DOUBLE , MPI_STATUS_IGNORE);
    }

    MPI_File_close( &file_handle);

    // FILE *out = fopen(filename, "wb");
    // if (out == NULL) {
    //     perror("fopen output file");
    //     fprintf(stderr, "Failed to open '%s' for writing.\n", filename);
    //     exit(EXIT_FAILURE);
    // }

    // for ( int_t i = 0; i < M; ++i ) {
    //     size_t written = fwrite ( &U(i,0), sizeof(real_t), (size_t)N, out );
    //     if ( written != (size_t)N ) {
    //         perror("fwrite");
    //         fclose(out);
    //         exit(EXIT_FAILURE);
    //     }
    // }

    // if ( fclose(out) != 0 ) {
    //     perror("fclose");
    //     exit(EXIT_FAILURE);
    // }
// END: T8
}


// TASK: T7
// Neumann (reflective) boundary condition
void boundary_condition ( void )
{
// BEGIN: T7
    // Vertical ghost layers left/Right
    //columns
    for (int_t i=0; i<number_of_rows; i++) {
        U(i,-1) = U(i,1);       // bottom ghost row <- mirror of row 1
        U(i,number_of_columns)  = U(i,number_of_columns-2);     // top ghost row <- mirror of row N-2
    }

    //bottom / top row.
    for (int_t j=0; j<number_of_columns; j++) {
        U(-1,j) = U(1,j);       // left ghost col <- mirror of col 1
        U(number_of_rows,j)  = U(number_of_rows-2,j);     // right ghost col <- mirror of col M-2
    }

    // Corner ghost cells (use ghost indices)
    U(-1,-1) = U(1,1);             // bottom-left
    U(-1,number_of_columns)  = U(1,number_of_columns-2);           // top-left
    U(number_of_rows,-1)  = U(number_of_rows-2,1);           // bottom-right
    U(number_of_rows,number_of_columns)   = U(number_of_rows-2,number_of_columns-2);         // top-right
// END: T7
}


// TASK: T4
// Set up our three buffers, and fill two with an initial perturbation
// and set the time step.
void domain_initialize ( void )
{
// BEGIN: T4
    number_of_rows = M/dimensions[1];
    number_of_columns = N/dimensions[0];

    index_column_start = my_cords[0]*number_of_columns;
    //pga vår merkelige cart layout.
    index_row_start = M - (my_cords[1]+1) * number_of_rows;

    buffers[0] = calloc ( (number_of_columns+2)*(number_of_rows+2),sizeof(real_t) );
    buffers[1] = calloc ( (number_of_columns+2)*(number_of_rows+2),sizeof(real_t) );
    buffers[2] = calloc ( (number_of_columns+2)*(number_of_rows+2),sizeof(real_t) );

    if ( !buffers[0] || !buffers[1] || !buffers[2] ) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }


    printf("My cartesian rank %d, my coordinates are %d, %d, my start row:%d my start column %d \n",
        cartesian_rank, my_cords[0], my_cords[1], index_row_start, index_column_start);
    // initialize interior (physical cells)
    // y_span fordi vi har y_span antall rader
    for ( int_t i=0; i<number_of_rows; i++ )
    {
        for ( int_t j=0; j<number_of_columns; j++ )
        {
            //Hvilken rad vi er på sier noe om y koordinaten vår.
            real_t dx_i = (index_row_start + i - M/2.0);
            real_t dy_j = (index_column_start + j - N/2.0);
            real_t delta = sqrt( (dx_i*dx_i) / (real_t)M + (dy_j*dy_j) / (real_t)N );
            U_prv(i,j) = U(i,j) = exp ( -4.0*delta*delta );
        }
    }
    //printf("Verdien i en ghost celle %ld", )
    // Set the time step (CFL) with a safety factor
    dt = 1.0 / ( c * sqrt( 1.0/(dx*dx) + 1.0/(dy*dy) ) );
    dt *= 0.9; // safety factor

    // Fill ghost cells once so they are valid before the first step
    boundary_condition();
// END: T4
}


// Get rid of all the memory allocations
void domain_finalize ( void )
{
    free ( buffers[0] );
    free ( buffers[1] );
    free ( buffers[2] );

}


// TASK: T5
// Integration formula
void time_step ( void )
{
// BEGIN: T5
    for (int i = 0; i < number_of_rows; i++)
    {
        for (int j = 0; j < number_of_columns; j++)
        {
            U_nxt(i,j) = -U_prv(i,j) + 2.0*U(i,j)
                     + (dt*dt*c*c)/(dx*dy) * (
                        U(i-1,j)+U(i+1,j)+U(i,j-1)+U(i,j+1)-4.0*U(i,j)
                     );
        }
    }
// END: T5
}

// TASK: T6
// Communicate the border between processes.
void border_exchange ( void )
{
// BEGIN: T6
int up, down, left, right;
    MPI_Cart_shift(cartesian_comm, 0, 1, &left, &right);
    MPI_Cart_shift(cartesian_comm, 1, 1, &up, &down);

    //Håper up og down er riktig rekkefølge
    // printf("min cartesian rank er %d, mine cords er %d, "
    //     "%d, min up nabo er %d, min ned nabo er %d \n",
    // cartesian_rank, my_cords[0], my_cords[1], up, down);
    
    // printf("min cartesian rank er %d, mine cords er %d, "
    //     "%d, min venstre nabo er %d, min høyre nabo er %d \n\n",
    // cartesian_rank, my_cords[0], my_cords[1], left, right);

    //Trenger ikke å sjekke for om man har nabo, fordi den returnerer MPI_PROC_NULL
    //Og om man sender til MPI_PROC_NULL så skjer absolutt ingenting.
    real_t left_column_send[number_of_rows];
    real_t right_column_send[number_of_rows];
    //Man går kolonne for kolonne først (i think)
    for(int i = 0; i<number_of_rows; i++){
        left_column_send[i] = U(i,0);
        right_column_send[i] = U(i, number_of_columns-1);
    }

    real_t right_column_recv[number_of_rows];
    real_t left_column_recv[number_of_rows];
    //sender venstre kolonne til venstre nabo, Vil mota resultat i høyre kolonne
    //Har null som recieve status idk man.
    MPI_Sendrecv( left_column_send , number_of_rows , MPI_INT64_T , left , 0 ,
         right_column_recv , number_of_rows , MPI_INT64_T , right , 0 , cartesian_comm , NULL);
    
    //Sender høyre kolonne som motas i venstre kolonne
    MPI_Sendrecv( right_column_send , number_of_rows , MPI_INT64_T , right , 0 ,
         left_column_recv , number_of_rows , MPI_INT64_T , left , 0 , cartesian_comm , NULL);

    //Skal prøve å bytte alt for å se hva som skjer.
    if(right!=MPI_PROC_NULL){
        for(int i =0; i<number_of_rows; i++){
            U(i, number_of_columns) = right_column_recv[i];
        }
    }
    if(left!=MPI_PROC_NULL){
        for(int i =0; i<number_of_rows; i++){
            U(i, -1) = left_column_recv[i];
        }
    }
    
    real_t top_row_send[number_of_columns];
    real_t bottom_row_send[number_of_columns];

    real_t top_row_recv[number_of_columns];
    real_t bottom_row_recv[number_of_columns];
    //Man går kolonne for kolonne først (i think)
    for(int j = 0; j<number_of_columns; j++){
        top_row_send[j] = U(number_of_rows-1,j);
        bottom_row_send[j] = U(0, j);
    }

    //Sender til ned nabo, mottar fra opp nabo
    MPI_Sendrecv( bottom_row_send , number_of_columns , MPI_DOUBLE , down , 0 ,
         top_row_recv , number_of_columns , MPI_DOUBLE , up , 0 , cartesian_comm , NULL);
    
    //Sender til opp nabo, motar fra ned nabo.
    MPI_Sendrecv( top_row_send , number_of_columns , MPI_DOUBLE , up , 0 ,
         bottom_row_recv , number_of_columns , MPI_DOUBLE , down , 0 , cartesian_comm , NULL);
    
    // if(up!=MPI_PROC_NULL){
    //     for(int j =0; j<number_of_rows; j++){
    //         U(-1, number_of_columns-j) = top_row_recv[j];
    //     }
    // }

    // if(down!=MPI_PROC_NULL){
    //     for(int j =0; j<number_of_rows; j++){
    //         U(number_of_rows, number_of_columns-j) = bottom_row_recv[j];
    //     }
    // }


// END: T6
}


// Main time integration.
void simulate( void )
{
    // Go through each time step
    for ( int_t iteration=0; iteration<=max_iteration; iteration++ )
    {
        if ( (iteration % snapshot_freq)==0 )
        {
            domain_save ( iteration / snapshot_freq );
        }

        // Derive step t+1 from steps t and t-1
        boundary_condition();
        border_exchange();
        time_step();

        // Rotate the time step buffers
        move_buffer_window();
    }
}


int main ( int argc, char **argv )
{
// TASK: T1c
// Initialise MPI
// BEGIN: T1c
    MPI_Init(&argc, &argv);
    ;
// END: T1c


// TASK: T3
// Distribute the user arguments to all the processes
// BEGIN: T3
    MPI_Comm_rank( MPI_COMM_WORLD , &world_rank);
    MPI_Comm_size( MPI_COMM_WORLD , &world_size);

    if (world_rank == 0) {
        OPTIONS *options = parse_args( argc, argv );
        if ( !options )
        {
            fprintf( stderr, "Argument parsing failed\n" );
            exit( EXIT_FAILURE );
        }

        M = options->M;
        N = options->N;
        max_iteration = options->max_iteration;
        snapshot_freq = options->snapshot_frequency;
    }
    //Prøver med å bare sende alle argumentene
    int_t argument_buffer[4] = {M,N,max_iteration,snapshot_freq};
    MPI_Bcast( &argument_buffer , 4 , MPI_INT64_T , 0 , MPI_COMM_WORLD);
    M = argument_buffer[0];
    N = argument_buffer[1];
    max_iteration = argument_buffer[2];
    snapshot_freq = argument_buffer[3];
// END: T3

    MPI_Dims_create( world_size , 2 , dimensions);
    //Periods er om den wrapper rundt, like mange elementer som dimensjoner
    const int periods[2] = {0,0};
    MPI_Cart_create( MPI_COMM_WORLD, 2 , dimensions , periods , 1 , &cartesian_comm);

    MPI_Comm_rank( cartesian_comm , &cartesian_rank);

    MPI_Cart_coords( cartesian_comm , cartesian_rank , 2 , my_cords);

    // Set up the initial state of the domain
    domain_initialize();
    
    MPI_Barrier(cartesian_comm);
    printf("My cartesian rank %d, my coordinates are %d, %d, my startx:%d my starty %d \n",
         cartesian_rank, my_cords[0], my_cords[1], index_row_start, index_column_start);
    // TASK: T2
    // Time your code
    // BEGIN: T2

    struct timeval t_start, t_end;
    if (gettimeofday(&t_start, NULL) < 0){
        printf("Noe gikk feil \n");
    };
    simulate();

    gettimeofday(&t_end, NULL);
    printf("from proc %d \n", world_rank);
    printf("Time spent in simulate: %lf seconds\n", WALLTIME(t_end) - WALLTIME(t_start));
// END: T2

    // Clean up and shut down
    domain_finalize();

// TASK: T1d
// Finalise MPI
// BEGIN: T1d
    MPI_Finalize();
    ;
// END: T1d

    exit ( EXIT_SUCCESS );
}
