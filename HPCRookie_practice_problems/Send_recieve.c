#include <stdio.h>
#include <mpi.h>

int main(int argc, char **argv){
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank( MPI_COMM_WORLD , &rank);
    MPI_Comm_size( MPI_COMM_WORLD, &size);

    int number = 0;

    if(rank == 0){
        number = 42;
    }
    if(rank == 1){
        number = 19;
    }
    int recieved = 0;
    //int mpi_status = 0;
    int partner = 1 - rank;
    printf("test før \n");
    MPI_Sendrecv( &number , 1 , MPI_INT , partner , 0 , &recieved , 1 , MPI_INT , partner , 0 , MPI_COMM_WORLD , MPI_STATUS_IGNORE);
    //printf("value of status %d", mpi_status);

    printf("value of number %d, from rank %d", rank, number);

    MPI_Finalize();

    return 0;
}