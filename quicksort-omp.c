#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "omp.h"
#define ARR_SIZE 65536
//#define ARR_SIZE 131072

// Funcao auxiliar para fazer o swap
void swap(int *arr, int i, int j)
{
    int temp = arr[i];
    arr[i] = arr[j];
    arr[j] = temp;
}

// Funcao que de fato preisa ser paralelizada
int partition(int *arr, int left, int right)
{
    int pivot = arr[right];
    int partitionIdx = left;
    for (int i = left; i < right; i++)
    {
        if (arr[i] < pivot)
        {
            swap(arr, partitionIdx, i);
            partitionIdx++;
        }
    }
    swap(arr, partitionIdx, right);
    return partitionIdx;
}

void quicksort(int *arr, int left, int right)
{
    if (left < right)
    {
        int partitionIdx = partition(arr, left, right);

        //Utilizamos as tasks o OMP para paralelizar as chamadas recursivas
        // Firstprivate : Especifica que cada thread deve ter sua própria instância de uma variável e que a variável deve ser inicializada com o valor da variável, pois ela existe antes da construção paralela.
        #pragma omp task firstprivate(arr,left,partitionIdx)
        {
            quicksort(arr, left, partitionIdx - 1);
        }
        {
            quicksort(arr, partitionIdx + 1, right);
        }
    }
}


void main()
{

    // Inicializa o array
    int* array = (int*)malloc(sizeof(int) * ARR_SIZE);
    // Pega o tamanho inicial do array
    int array_size = ARR_SIZE;

    // Leitur do array do arquivo
    FILE *f = fopen("array65k", "rb");
    fread(array, sizeof(int), ARR_SIZE, f);
    fclose(f);

    // inicio do programa paralelo 
    #pragma omp parallel num_threads(2)
    {
        double start = omp_get_wtime();

        // indica q esse codigo sera rodado por apenas uma thread
        #pragma omp single nowait
        {
            quicksort(array, 0, ARR_SIZE);
        }

    // Calculo de tempo de execucao
    double end = omp_get_wtime();
    printf("The elapsed time is %lf seconds", end-start);
    }


    return;
}