#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main()
{
    int array_size = 0;
    int *arr = NULL;
    FILE *fp = NULL;

    // 1. Чтение размера массива из файла
    fp = fopen("array_size.txt", "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Error: couldn't open the file array_size.txt\n");
        return 1;
    }

    if (fscanf(fp, "%d", &array_size) != 1)
    {
        fprintf(stderr, "Error: couldn't read the size of the array from the file\n");
        fclose(fp);
        return 1;
    }

    fclose(fp);

    if (array_size <= 100000)
    {
        fprintf(stderr, "Error: the size of the array must be larger 100000\n");
        return 1;
    }

    // 2. Выделение памяти для массива
    arr = (int *)malloc(array_size * sizeof(int));
    if (arr == NULL)
    {
        fprintf(stderr, "Memory allocation error in the process 0\n");
        return 1;
    }

    // 3. Инициализация массива случайными числами
    srand(time(NULL));
    for (int i = 0; i < array_size; i++)
    {
        arr[i] = rand() % 1000; // Случайные числа от 0 до 999
    }

    // 4. Сортировка пузырьком
    clock_t start_time = clock();

    for (int i = 0; i < array_size - 1; i++)
    {
        for (int j = 0; j < array_size - i - 1; j++)
        {
            if (arr[j] > arr[j + 1])
            {
                // Обмен элементов
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }

    clock_t end_time = clock();

    // 5. Вывод времени сортировки
    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("Execution time: %.6f sec\n", elapsed_time);

    // 6. Освобождение памяти
    free(arr);

    return 0;
}