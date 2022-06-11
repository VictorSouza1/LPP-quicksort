#include <stdio.h>
#include "mpi.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "header.h"

int main(argc, argv)
int argc;
char **argv;
{
    // initializing control variables
    int slice_size, num_steps, array_size, pivot, division, partner;
    int array[160];

    // Initialize MPI and get rank and size
    MPI_Status status;
    MPI_Init(&argc, &argv);
    int size;
    int rank;
    int tag = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // O processo zero vai ser o master e vai controlar cada passo do algo
    if (rank == 0)
    {
        // initializing the array
        array_size = 160;
        // int array[40] = {13, 38, 16, 17, 39, 11, 37, 12, 21, 19, 24, 36, 34, 8, 35, 25, 9, 30, 5, 1, 32, 31, 22, 6, 40, 28, 3, 20, 2, 18, 14, 7, 33, 26, 23, 29, 15, 4, 27, 10};
        //int array[16] = {10, 118, 146, 120, 27, 5, 58, 8, 12, 6, 137, 112, 119, 13, 43, 84, 67, 4, 115, 31, 19, 128, 113, 24, 86, 46, 77, 104, 61, 121, 106, 101, 143, 133, 102, 40, 80, 62, 78, 72, 44, 36, 28, 158, 49, 32, 108, 91, 94, 99, 53, 22, 65, 92, 140, 60, 25, 42, 134, 79, 114, 95, 57, 93, 131, 123, 139, 132, 153, 144, 38, 160, 23, 56, 97, 135, 59, 107, 149, 82, 45, 136, 141, 124, 21, 157, 148, 66, 145, 14, 88, 155, 129, 151, 89, 138, 122, 69, 50, 105, 156, 68, 100, 15, 159, 18, 39, 34, 20, 74, 110, 51, 116, 3, 41, 1, 33, 111, 71, 126, 75, 152, 48, 117, 63, 2, 16, 70, 142, 55, 85, 35, 9, 103, 64, 83, 26, 7, 130, 76, 47, 73, 11, 125, 154, 96, 109, 54, 29, 150, 98, 81, 87, 30, 127, 17, 37, 147, 90, 52};
        for(int i=0;i<array_size;i++)
            array[i]=rand()%100;  //Generate number between 0 to 99
    }

    // sending slices of the array to each proccess
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Bcast(&array_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    // printf("Sou o processo %d e recebi o tamanho %d do array\n", rank, array_size);
    slice_size = array_size / size;
    int slice[array_size];
    int recive_slice[array_size];
    MPI_Scatter(array, slice_size, MPI_INT, slice, slice_size, MPI_INT, 0, MPI_COMM_WORLD);
    // printf("Sou o processo %d e estou recebendo o array com o primeiro elemento %d do scatter\n", rank, slice[0]);
    //  o numero de passos do algoritimo vai ser log(threads) na base 2
    num_steps = log10(size) / log10(2);
    // Fim da fase de inicializacao, agora vem a parte que sera repetida

    // --------------------------------------------------- // ----------------------------------------------------------

    MPI_Barrier(MPI_COMM_WORLD);
    for (int step = 0; step < num_steps; step++)
    {
        // Criando novo(s) cominicador(es) separando cada processo dependo da iteracao que estamos
        int iteration_rank, iteration_size;
        int iteration_comm_id = pow(2, step) * rank / size;
        MPI_Comm MPI_COMM_ITERATION;
        MPI_Comm_split(MPI_COMM_WORLD, iteration_comm_id, rank, &MPI_COMM_ITERATION);
        MPI_Comm_rank(MPI_COMM_ITERATION, &iteration_rank);
        MPI_Comm_size(MPI_COMM_ITERATION, &iteration_size);

        // printf("Sou o processo com rank global %d, estou na iteracao %d e estou no comunicador de iteralcao %d de tamanho %d e com o rank de iteracao %d \n", rank, step, iteration_comm_id, iteration_size, iteration_rank);

        // Cada processo com o rank zero manda o pivot para todos o processos desse novo Comunicador
        if (iteration_rank == 0)
        {
            // Escolha do pivot aleatoria (aqui pegamos o primeiro elemento do vetor), problema, e se o vetor tiver vazio ?????
            pivot = slice[0];
        }
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Bcast(&pivot, 1, MPI_INT, 0, MPI_COMM_ITERATION);
        // printf("Iteracao: %d  | Sou o processo com rank global %d, e estou no comunicador de iteracao %d de tamanho %d e com o rank de iteracao %d e recebi o pivot: %d \n", step, rank, iteration_comm_id, iteration_size, iteration_rank, pivot);

        // Cada processo faz a divisao do seu array entre maiores e menores que pivot, e armazena em division o local dessa separacao
        division = partition(slice, pivot, slice_size);
        // for(int i = 0; i < slice_size; i++){
        //     printf("Processo %d, slice[%d]=%d \n", rank, i ,slice[i]);
        // }
        // printf("Processo %d Division = %d", rank, division);
        // printf("\n");

        // Descobrir o processo para parear
        if (iteration_rank < iteration_size / 2)
        {
            partner = iteration_size / 2 + iteration_rank;
        }
        else
        {
            partner = iteration_rank - iteration_size / 2;
        }
        // printf("Sou o processo %d pareado com o processo %d\n", rank, partner);

        // Processos parceiros trocam partes dos seus arrays
        if (iteration_rank < iteration_size / 2)
        {
            MPI_Send(slice + division, slice_size - division, MPI_INT, partner, 0, MPI_COMM_ITERATION);
            MPI_Recv(recive_slice, array_size, MPI_INT, partner, 0, MPI_COMM_ITERATION, &status);
        }
        else
        {
            MPI_Recv(recive_slice, array_size, MPI_INT, partner, 0, MPI_COMM_ITERATION, &status);
            MPI_Send(slice, division, MPI_INT, partner, 0, MPI_COMM_ITERATION);
        }

        int recive_slice_size;
        MPI_Get_count(&status, MPI_INT, &recive_slice_size);

        // for(int i = 0; i < recive_slice_size; i++){
        //     printf("Debug Processo %d, slice_recived[%d]=%d \n", rank, i ,recive_slice[i]);
        // }

        // Removendo partes nao mais utilizadas do array slice de cada processo
        // E adicionando novos elementos recebidos pelos pares
        if (iteration_rank < iteration_size / 2)
        {
            for (int i = 0; i < recive_slice_size; i++)
            {
                slice[division + i] = recive_slice[i];
            }
            slice_size = division + recive_slice_size;
        }
        else
        {
            for (int i = 0; i < recive_slice_size; i++)
            {
                slice[slice_size + i] = recive_slice[i];
            }
            slice_size = slice_size + recive_slice_size;
            // TODO melhorar a logica de remove usando buffer circular
            removeElementsFromBeginingOfArray(slice, division, slice_size);
            slice_size = slice_size - division;
        }

        for (int i = 0; i < slice_size; i++)
        {
            printf("Final Step = %d | Processo %d, slice[%d]=%d \n", step, rank, i, slice[i]);
        }

        MPI_Comm_free(&MPI_COMM_ITERATION);
        MPI_Barrier(MPI_COMM_WORLD);
    }

    // Cada processo performa um quicksort sequencial em cada um de seus slices
    quicksort_sequential(slice, 0, slice_size-1);

    // Fase de recuperar todos os arrays de cada processo e juntar no master
    if(rank == 0) {
        int sizes_of_slices[size];
        int displacments[size];
        MPI_Gather(&slice_size, 1, MPI_INT, sizes_of_slices, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);
        calculateDisplacmentArray(sizes_of_slices, displacments, size);
        MPI_Gatherv(slice, slice_size, MPI_INT, array, sizes_of_slices, displacments, MPI_INT, 0, MPI_COMM_WORLD);

    }
    else {
        MPI_Gather(&slice_size, 1, MPI_INT, NULL, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Gatherv(slice, slice_size, MPI_INT, array, NULL, NULL, MPI_INT, 0, MPI_COMM_WORLD);
    }



    printf("\n");

    if (rank == 0)
    {
        for (int i = 0; i < array_size; i++)
        {

            printf("%d,", array[i]);
        }
    }



    MPI_Finalize();
    return 0;
}