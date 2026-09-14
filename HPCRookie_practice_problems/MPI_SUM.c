#include <stdio.h>
#include <mpi.h>

int main(int argc, char **argv){

    MPI_Init(&argc, &argv);

    int rank, size;

    MPI_Comm_rank( MPI_COMM_WORLD , &rank);
    MPI_Comm_size( MPI_COMM_WORLD , &size);

    int number = 100 * rank;
    int result = 0;

    MPI_Reduce( &number , &result , 1 , MPI_INT , MPI_SUM , 0 , MPI_COMM_WORLD);

    printf("I am process %d     value of result %d \n", rank, result);

    MPI_Finalize();

    return 0;
}