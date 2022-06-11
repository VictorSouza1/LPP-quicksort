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
    // Inicialzando variaveis de controle
    int slice_size, num_steps, array_size, pivot, division, partner;
    double tempo_inicial, tempo_final;
    int* array = (int*)malloc(sizeof(int) * 1048576);

    // Initialize MPI and get rank and size
    MPI_Status status;
    MPI_Init(&argc, &argv);
    int size;
    int rank;
    int tag = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // O processo zero vai ser o master e vai controlar inicializar e enviar o array para os outros participantes
    if (rank == 0)
    {
        // inicializando o array
        array_size = 1048576; // Mudar aqui o numero de elementos do array
        for(int i=0;i<array_size;i++)
            array[i]=rand()%100000;  //Se o array for muito grante pode aumentar esse nr
    }

    MPI_Barrier(MPI_COMM_WORLD);

    tempo_inicial = MPI_Wtime();
    // O processo 0 manda o tamanho de cada slice para todos os processos
    MPI_Bcast(&array_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    slice_size = array_size / size;
    //Cada array inicializa seu slie
    int* slice = (int*)malloc(sizeof(int) * array_size);

    int* recive_slice = (int*)malloc(sizeof(int) * array_size); // Esse array vai ser util no futuro

    // O processo 0 envia os slices para os processo
    MPI_Scatter(array, slice_size, MPI_INT, slice, slice_size, MPI_INT, 0, MPI_COMM_WORLD);

    //  Calculando o numero de passos do algoritmo (log(processos) na base 2)
    num_steps = log10(size) / log10(2);
    // Fim da fase de inicializacao

    // --------------------------------------------------- // ----------------------------------------------------------    

    MPI_Barrier(MPI_COMM_WORLD);
    for (int step = 0; step < num_steps; step++)
    {
        // Criando novo(s) cominicador(es) separando cada processo em seu devido comunicador
        // dependo da iteracao que estamos
        int iteration_rank, iteration_size;
        // Cada processo descobre qual comunicador ele pertence
        int iteration_comm_id = pow(2, step) * rank / size;
        MPI_Comm MPI_COMM_ITERATION;
        // Aqui e como o mpi lida com essa criacao dinamica de comunicadores, splitando um comunicador em varios
        MPI_Comm_split(MPI_COMM_WORLD, iteration_comm_id, rank, &MPI_COMM_ITERATION);
        MPI_Comm_rank(MPI_COMM_ITERATION, &iteration_rank);
        MPI_Comm_size(MPI_COMM_ITERATION, &iteration_size);


        // Cada processo com o rank zero no seu comunicador manda o pivot para todos o processos desse novo Comunicador
        if (iteration_rank == 0)
        {
            // Escolha do pivot aleatoria (aqui pegamos o primeiro elemento do vetor), problema, e se o vetor tiver vazio ?????, nao tratei isso
            pivot = slice[0];
        }
        MPI_Barrier(MPI_COMM_WORLD);

        // O processo 0 manda o pivot pros processos do seu comunicador
        MPI_Bcast(&pivot, 1, MPI_INT, 0, MPI_COMM_ITERATION);

        // Cada processo faz a particao do seu array entre maiores e menores que pivot, e armazena em division o local dessa separacao
        division = partition(slice, pivot, slice_size);

        // Descobrir o processo para parear (essa parte fica facil com a diviao de comunicadores feita)
        if (iteration_rank < iteration_size / 2)
        {
            partner = iteration_size / 2 + iteration_rank;
        }
        else
        {
            partner = iteration_rank - iteration_size / 2;
        }

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
        // Rotina mpi para pegar o tamanho do array recevido no MPI_Recv anterior 
        //(para evitar de fazer uma comuncacao amais so pra mandar o tamanho do array)
        MPI_Get_count(&status, MPI_INT, &recive_slice_size);


        // Processos da primeira metade vao descartar os elementtos maiores do que o pivot e adicionar
        // Os ementso recebidos pelo seu par
        if (iteration_rank < iteration_size / 2)
        {
            for (int i = 0; i < recive_slice_size; i++)
            {
                slice[division + i] = recive_slice[i];
            }
            slice_size = division + recive_slice_size;
        }
        else //Processos da segunda metade vao descartar os elementos menores do que o pivot e adicionar o elementos recebidos
        {
            for (int i = 0; i < recive_slice_size; i++)
            {
                slice[slice_size + i] = recive_slice[i];
            }
            slice_size = slice_size + recive_slice_size;
            slice+=division;
            slice_size = slice_size - division;
        }

        // Destroy os comunicadores criados para esse step
        MPI_Comm_free(&MPI_COMM_ITERATION);
        MPI_Barrier(MPI_COMM_WORLD);
    }

    // Cada processo performa um quicksort sequencial em cada um de seus slices
    quicksort_sequential(slice, 0, slice_size-1);

    // Fase de recuperar todos os arrays de cada processo e juntar no master
    // Utilizando o MPI_Gatherv para juntar arrays de diferentes sizes
    if(rank == 0) {
        int* sizes_of_slices = (int*)malloc(sizeof(int) * size);
        int* displacments = (int*)malloc(sizeof(int) * size);
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

    tempo_final = MPI_Wtime();


    printf("\n");
    // Printa o array ordenado
    if (rank == 0)
    {
        printf("Levou %lf\n", tempo_final - tempo_inicial);
        // for (int i = 0; i < array_size; i++)
        // {

        //     printf("%d,", array[i]);
        // }
    }



    MPI_Finalize();
    return 0;
}