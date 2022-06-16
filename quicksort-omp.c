#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "omp.h"

void swap(int *arr, int i, int j)
{
    int temp = arr[i];
    arr[i] = arr[j];
    arr[j] = temp;
}


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

    clock_t start, end;
    double cpu_time_used;


    int* array = (int*)malloc(sizeof(int) * 33554432);
    int array_size = 33554432;
    for(int i=0;i<array_size;i++)
        array[i]=rand()%100000; 

    start = clock();
    #pragma omp parallel num_threads(8)
    {
        #pragma omp single nowait
        {
            quicksort(array, 0, 33554431);
        }
    }
    end = clock();

    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;




    printf("The elapsed time is %lf seconds", cpu_time_used);

    return;
}