#include <stdio.h>
#include "mpi.h"
#include <math.h>
#include <string.h>

void swap(int *arr, int i, int j)
{
    int temp = arr[i];
    arr[i] = arr[j];
    arr[j] = temp;
}

int partition(int *arr, int pivot, int size)
{
    int aux = 0;
    for (int i = 0; i < size; i++)
    {
        if (arr[i] <= pivot)
        {
            swap(arr, i, aux);
            aux++;
        }
    }
    return aux;
}


void calculateDisplacmentArray(int *sizes, int *displacments, int size)
{
    int sum = 0;
    for(int i = 0; i<size; i++) {
        displacments[i] = sum;
        sum+=sizes[i];
    }
}



void swap_sequential(int *arr, int i, int j)
{
    int temp = arr[i];
    arr[i] = arr[j];
    arr[j] = temp;
}


int partition_sequential(int *arr, int left, int right)
{
    int pivot = arr[right];
    int partitionIdx = left;
    for (int i = left; i < right; i++)
    {
        if (arr[i] < pivot)
        {
            swap_sequential(arr, partitionIdx, i);
            partitionIdx++;
        }
    }
    swap_sequential(arr, partitionIdx, right);
    return partitionIdx;
}

void quicksort_sequential(int *arr, int left, int right)
{
    if (left < right)
    {
        int partitionIdx = partition_sequential(arr, left, right);
        quicksort_sequential(arr, left, partitionIdx - 1);
        quicksort_sequential(arr, partitionIdx + 1, right);
    }
}