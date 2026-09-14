#include <stdio.h>
#include <string.h>
//NOLINTNEXTLINE
#include <mpi.h>

int main(int argc, char **argv){
    MPI_Init(&argc, &argv);

    int myRank, n_procs;
    n_procs = MPI_Comm_size( MPI_COMM_WORLD , &n_procs);

    MPI_Comm_rank( MPI_COMM_WORLD , &myRank);

    if(myRank == 0){
        printf("I am the first process my rank is %d \n", myRank);

        int number_to_send = 42;

        MPI_Send( &number_to_send, 1 , MPI_INT , 1 , 111 , MPI_COMM_WORLD);

        //Skal nå sende en streng.
        //
        char* message = "message from rank 0 to rank 1 \n";

        int length = strlen(message);
        printf("%d \n", length);

        MPI_Send( message , length+1 , MPI_CHAR , 1 ,  0 , MPI_COMM_WORLD);
    }
    if(myRank ==1){
        int number_recieved;

        MPI_Recv( &number_recieved , 1 , MPI_INT , 0 , 111 , MPI_COMM_WORLD , MPI_STATUS_IGNORE);
        
        printf("Number recieved %d \n", number_recieved);
        
        char inbox[32];
        
        MPI_Recv( inbox , 32 , MPI_CHAR , 0 , 0 , MPI_COMM_WORLD , MPI_STATUS_IGNORE);

        printf("%s", inbox);

    }

    MPI_Finalize();
    return 0;
}