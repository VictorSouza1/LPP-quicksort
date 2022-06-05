#include <stdio.h>
#include "mpi.h"
#include <math.h>

int main(argc, argv)
int argc;
char **argv;
{
    // initializing control variables
    int slice_size, num_steps, array_size, pivot;
    int array[16];
    
    // Initialize MPI and get rank and size
    MPI_Init(&argc, &argv);
    int size;
    int rank;
    int tag = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // O processo zero vai ser o master e vai controlar cada passo do algo
    if(rank == 0) {
        //initializing the array
        array_size = 16;
        //int array[40] = {13, 38, 16, 17, 39, 11, 37, 12, 21, 19, 24, 36, 34, 8, 35, 25, 9, 30, 5, 1, 32, 31, 22, 6, 40, 28, 3, 20, 2, 18, 14, 7, 33, 26, 23, 29, 15, 4, 27, 10};
        int array[16] = {19,5,11,2,8,17,12,16,10,4,25,13,18,15,3,1};

    }

    // sending slices of the array to each proccess
    MPI_Bcast(&array_size , 1 , MPI_INT , 0 , MPI_COMM_WORLD);
    //printf("Sou o processo %d e recebi o tamanho %d do array\n", rank, array_size);
    MPI_Barrier(MPI_COMM_WORLD);
    slice_size = array_size / size;
    int slice[array_size];
    MPI_Scatter( array , slice_size , MPI_INT , slice , slice_size , MPI_INT , 0 , MPI_COMM_WORLD);
    //printf("Sou o processo %d e estou recebendo o array com o primeiro elemento %d do scatter\n", rank, slice[0]);
    // o numero de passos do algoritimo vai ser log(threads) na base 2
    num_steps = log10(size) / log10(2); 
    // Fim da fase de inicializacao, agora vem a parte que sera repetida 
    

    // --------------------------------------------------- // ----------------------------------------------------------

    MPI_Barrier( MPI_COMM_WORLD);
    for(int step=0; step< num_steps; step++) {
        // Criando novo(s) cominicador(es) separando cada processo dependo da iteracao que estamos
        int iteration_rank, iteration_size;
        int iteration_comm_id = pow(2,step)*rank/size;
        MPI_Comm MPI_COMM_ITERATION;
        MPI_Comm_split( MPI_COMM_WORLD , iteration_comm_id , rank , &MPI_COMM_ITERATION);
        MPI_Comm_rank(MPI_COMM_ITERATION, &iteration_rank);
        MPI_Comm_size(MPI_COMM_ITERATION, &iteration_size);

        printf("Sou o processo com rank global %d, estou na iteracao %d e estou no comunicador de iteralcao %d de tamanho %d e com o rank de iteracao %d \n", rank, step, iteration_comm_id, iteration_size, iteration_rank);
        
        // Cada processo com o rank zero manda o pivot para todos o processos desse novo Comunicador
        MPI_Bcast( &pivot , 1 , MPI_INT , 0 , MPI_COMM_ITERATION);
        MPI_Barrier( MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}