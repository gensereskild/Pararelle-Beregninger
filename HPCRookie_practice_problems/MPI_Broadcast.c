#include <stdio.h>
#include <string.h>
#include <mpi.h>

int main(int argc, char **argv){
    MPI_Init(&argc, &argv);

    int rank, size;

    MPI_Comm_rank( MPI_COMM_WORLD, &rank);
    MPI_Comm_size( MPI_COMM_WORLD , &size);

    if(size==11){
        printf("You cannot start with 11 proceses for whatever reason \n");
        MPI_Abort( MPI_COMM_WORLD , 1);
    }

    char storage[1000];
    if(rank == 0){
        strcpy(storage, "Broadcast to all procesess");
    }
    
    MPI_Bcast( storage , sizeof(storage)+1 , MPI_CHAR , 0 , MPI_COMM_WORLD);

    printf("Did i someone but something in my buffer?: %s \n", storage);


    MPI_Finalize();

    return 0;
}