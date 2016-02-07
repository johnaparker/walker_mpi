#include <iostream>
#include <mpi.h>

using namespace std;

void doSomething() {
    //MPI_Init(&argc, &argv);
    MPI_Init(NULL,NULL);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    double value = 1;
    double final_value;
    MPI_Status status;

    if (world_rank == 0) {
        for (int i = 0; i != 1000000000/2; i ++) {
            value += i/1000.0;
        }
        MPI_Recv(&final_value, 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD, &status);
        final_value += value;
        cout << final_value << endl;
    }
    else {
        for (int i = 1000000000/2; i != 1000000000; i ++) {
            value += i/1000.0;
        }
        MPI_Send(&value, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }
    MPI_Finalize();
}

int main(int argc, char** argv) {


    doSomething();


}
