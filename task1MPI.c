#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

int main(int argc, char **argv)
{
    int rank, size;
    int array_size = 0;
    int *arr = NULL;
    int local_size = 0;
    int *local_arr = NULL;
    long long global_sum = 0, local_sum = 0;
    double start_time, end_time, total_time;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0)
    {
        FILE *fp = fopen("array_size.txt", "r"); // Файл с размером массива
        if (fp == NULL)
        {
            fprintf(stderr, "Error: couldn't open the file array_size.txt\n");
            MPI_Abort(MPI_COMM_WORLD, 1); // Завершаем MPI программу с кодом ошибки 1
        }
        if (fscanf(fp, "%d", &array_size) != 1)
        {
            fprintf(stderr, "Error: couldn't read the size of the array from the file\n");
            fclose(fp);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        fclose(fp);

        if (array_size <= 100000)
        {
            fprintf(stderr, "Error: the size of the array must be larger 100000\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        arr = (int *)malloc(array_size * sizeof(int));
        if (arr == NULL)
        {
            fprintf(stderr, "Memory allocation error in the process 0\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        // Инициализируем массив случайными числами
        srand(time(NULL));
        for (int i = 0; i < array_size; i++)
        {
            arr[i] = rand() % 100;
        }
    }

    MPI_Bcast(&array_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    local_size = array_size / size;
    if (array_size % size != 0 && rank == size - 1)
    {
        local_size += array_size % size;
    }

    local_arr = (int *)malloc(local_size * sizeof(int));
    if (local_arr == NULL)
    {
        fprintf(stderr, "Memory allocation error in the process %d\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    int *sendcounts = NULL, *displs = NULL;
    if (rank == 0)
    {
        sendcounts = (int *)malloc(size * sizeof(int));
        displs = (int *)malloc(size * sizeof(int));
        int base_size = array_size / size;
        int remainder = array_size % size;

        displs[0] = 0;
        sendcounts[0] = base_size + (0 < remainder ? 1 : 0);

        for (int i = 1; i < size; i++)
        {
            displs[i] = displs[i - 1] + sendcounts[i - 1];
            sendcounts[i] = base_size + (i < remainder ? 1 : 0);
        }
    }

    MPI_Scatterv(arr, sendcounts, displs, MPI_INT,
                 local_arr, local_size, MPI_INT,
                 0, MPI_COMM_WORLD);

    start_time = MPI_Wtime();

    local_sum = 0;
    for (int i = 0; i < local_size; i++)
    {
        local_sum += local_arr[i];
    }

    MPI_Reduce(&local_sum, &global_sum, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    end_time = MPI_Wtime();
    total_time = end_time - start_time;

    if (rank == 0)
    {
        printf("The sum of the array elements: %lld\n", global_sum);
        printf("Execution time (in parallel with MPI): %.6f sec\n", total_time);
    }

    if (rank == 0)
    {
        free(arr);
        free(sendcounts);
        free(displs);
    }
    free(local_arr);

    MPI_Finalize();
    return 0;
}