#include <stdio.h>
#include <time.h>
#include <stdlib.h>

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
        quicksort(arr, left, partitionIdx - 1);
        quicksort(arr, partitionIdx + 1, right);
    }
}




void main()
{

    clock_t start, end;
    double cpu_time_used;


    int* array = (int*)malloc(sizeof(int) * 1048576);
    int array_size = 1048576;
    for(int i=0;i<array_size;i++)
        array[i]=rand()%100000; 

    start = clock();
    quicksort(array, 0, 1048575);
    end = clock();

    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;




    printf("The elapsed time is %lf seconds", cpu_time_used);

    printf("[");
    for(int i = 0; i < 6; i++){
        printf("%d", array[i]);
        if(i != 5) {
            printf(",");
        }
    }
    printf("]\n");
    return;
}