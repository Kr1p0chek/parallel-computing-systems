#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

bool is_sorted(int* arr, int n) {
    for (int i = 0; i < n - 1; i++) {
        if (arr[i] > arr[i + 1]) return false;
    }
    return true;
}

int main(int argc, char* argv[])
{
    int array_size = 0;
    int* array = NULL;

    if (argc != 2) {
        printf("Usage: %s <array_size\n", argv[0]);
        return 1;
    }

    array_size = atoi(argv[1]);
    if (array_size <= 10000)
    {
        fprintf(stderr, "Error: Array size must be greater than 10000\n");
        return 1;
    }

    array = (int*)malloc(array_size * sizeof(int));
    if (array == NULL)
    {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return 1;
    }

    srand(time(NULL));
    for (int i = 0; i < array_size; i++)
    {
        array[i] = rand() % 1000; // Случайные числа от 0 до 999
    }
    clock_t start_time = clock();

    for (int i = 0; i < array_size - 1; i++)
    {
        for (int j = 0; j < array_size - i - 1; j++)
        {
            if (array[j] > array[j + 1])
            {
                // Обмен элементов
                int temp = array[j];
                array[j] = array[j + 1];
                array[j + 1] = temp;
            }
        }
    }

    clock_t end_time = clock();

    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    if (!is_sorted(array, array_size)) {
        printf("Sort error");
        free(array);
        return 1;
    }
    FILE* f = fopen("ser_sort_time.txt", "a");
    if (f == NULL) {
        printf("Can not open out file");
        free(array);
        return 1;
    }
    fprintf(f, "%lf\n", elapsed_time);
    fclose(f);
    free(array);

    return 0;
}
