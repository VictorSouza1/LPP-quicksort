#include <stdio.h>

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

    int arr[6] = { 5, 3, 1, 6, 4, 2 };
    int size = 6;

    quicksort(arr, 0, 5);


    printf("[");
    for(int i = 0; i < 6; i++){
        printf("%d", arr[i]);
        if(i != 5) {
            printf(",");
        }
    }
    printf("]\n");
    return;
}