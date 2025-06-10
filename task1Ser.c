#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main()
{
    int array_size = 0;
    int *arr = NULL;
    FILE *fp = NULL;
    long long sum = 0;

    fp = fopen("array_size.txt", "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Error: Could not open file array_size.txt\n");
        return 1;
    }

    if (fscanf(fp, "%d", &array_size) != 1)
    {
        fprintf(stderr, "Error: Could not read array size from file\n");
        fclose(fp);
        return 1;
    }

    fclose(fp);

    if (array_size <= 100000)
    {
        fprintf(stderr, "Error: Array size must be greater than 100000\n");
        return 1;
    }

    arr = (int *)malloc(array_size * sizeof(int));
    if (arr == NULL)
    {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return 1;
    }

    srand(time(NULL));
    for (int i = 0; i < array_size; i++)
    {
        arr[i] = rand() % 100;
    }

    clock_t start_time = clock();

    for (int i = 0; i < array_size; i++)
    {
        sum += arr[i];
    }

    clock_t end_time = clock();

    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("Sum of array elements: %lld\n", sum);
    printf("Execution time (sequential): %.6f seconds\n", elapsed_time);

    free(arr);

    return 0;
}