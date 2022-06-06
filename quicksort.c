#include <stdio.h>
#include "mpi.h"
#include <math.h>


void swap(int* arr, int i, int j) {
    int temp = arr[i];
    arr[i] = arr[j];
    arr[j] = temp;
}

int partition(int* arr, int pivot, int size) {
    int aux = 0;
    for (int i = 0; i < size; i++) {
        if(arr[i] <= pivot) {
            swap(arr, i, aux);
            aux++;
        }
    }
    return aux;
}

int main(argc, argv)
int argc;
char **argv;
{
    // initializing control variables
    int slice_size, num_steps, array_size, pivot, division, partner;
    int array[16];
    
    // Initialize MPI and get rank and size
    MPI_Status status;
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
        int array[16] = {5,11,19,2,8,17,12,16,10,4,25,13,18,15,3,1};

    }

    // sending slices of the array to each proccess
    MPI_Bcast(&array_size , 1 , MPI_INT , 0 , MPI_COMM_WORLD);
    //printf("Sou o processo %d e recebi o tamanho %d do array\n", rank, array_size);
    MPI_Barrier(MPI_COMM_WORLD);
    slice_size = array_size / size;
    int slice[array_size];
    int recive_slice[array_size];
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

        //printf("Sou o processo com rank global %d, estou na iteracao %d e estou no comunicador de iteralcao %d de tamanho %d e com o rank de iteracao %d \n", rank, step, iteration_comm_id, iteration_size, iteration_rank);
        
        // Cada processo com o rank zero manda o pivot para todos o processos desse novo Comunicador
        if(iteration_rank == 0){
            // Escolha do pivot aleatoria (aqui pegamos o primeiro elemento do vetor), problema, e se o vetor tiver vazio ?????
            pivot = slice[0]; 
        }
        MPI_Bcast(&pivot , 1 , MPI_INT , 0 , MPI_COMM_ITERATION);
        //printf("Iteracao: %d  | Sou o processo com rank global %d, e estou no comunicador de iteracao %d de tamanho %d e com o rank de iteracao %d e recebi o pivot: %d \n", step, rank, iteration_comm_id, iteration_size, iteration_rank, pivot);

        MPI_Barrier( MPI_COMM_WORLD);
        
        // Cada processo faz a divisao do seu array entre maiores e menores que pivot, e armazena em division o local dessa separacao
        division = partition(slice, pivot, slice_size);
        for(int i = 0; i < slice_size; i++){
            printf("Processo %d, slice[%d]=%d \n", rank, i ,slice[i]);
        }
        printf("Processo %d Division = %d", rank, division);
        printf("\n");

        MPI_Barrier( MPI_COMM_WORLD);

        // Descobrir o processo para parear
        if(iteration_rank < iteration_size/2) {
            partner = iteration_size/2 + iteration_rank;
        } else {
            partner = iteration_rank - iteration_size/2;
        }
        printf("Sou o processo %d pareado com o processo %d\n", rank, partner);

        // Processos parceiros trocam partes dos seus arrays
        if (iteration_rank < iteration_size/2) {
            MPI_Send(slice+division, slice_size-division, MPI_INT , partner, 0, MPI_COMM_ITERATION);
            MPI_Recv(recive_slice, array_size, MPI_INT, partner, 0, MPI_COMM_ITERATION, &status);
        } else {
            MPI_Recv(recive_slice, array_size, MPI_INT, partner, 0, MPI_COMM_ITERATION, &status);
            MPI_Send(slice, division, MPI_INT , partner, 0, MPI_COMM_ITERATION);
        }

        int recive_slice_size;
        MPI_Get_count(&status, MPI_INT, &recive_slice_size);
        printf("Sou o processo %d e recebi um recive_slice de tamanho %d com primeiro elemento %d\n", rank, recive_slice_size, recive_slice[0]);
        

        MPI_Barrier(MPI_COMM_WORLD);
    }


    MPI_Finalize();
    return 0;
}