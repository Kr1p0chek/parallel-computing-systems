#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank, size;
    int array_size = 0;
    int* array = NULL;
    int local_size = 0;
    int* local_array = NULL;
    long long global_sum = 0, local_sum = 0;
    double start_time, end_time, total_time;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);


    if (argc != 2) {
        if (rank == 0) {
            printf("Usage: %s <array_size>\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }

    array_size = atoi(argv[1]);
    if (array_size <= 100000)
    {
        if (rank == 0) {
            printf(stderr, "Error: the size of the array must be larger 100000\n");
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
        return 1;
    }

    if (rank == 0) {
        array = (int*)malloc(array_size * sizeof(int));
        if (array == NULL) {
            printf("Memmory allocation failed\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        srand(time(NULL));
        for (int i = 0; i < array_size; i++)
        {
            array[i] = rand() % 100;
        }
    }

    start_time = MPI_Wtime();

    MPI_Bcast(&array_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    local_size = array_size / size;
    if (array_size % size != 0 && rank == size - 1)
    {
        local_size += array_size % size;
    }

    local_array = (int *)malloc(local_size * sizeof(int));
    if (local_array == NULL)
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

    MPI_Scatterv(array, sendcounts, displs, MPI_INT,
                 local_array, local_size, MPI_INT,
                 0, MPI_COMM_WORLD);


    local_sum = 0;
    for (int i = 0; i < local_size; i++)
    {
        local_sum += local_array[i];
    }

    MPI_Reduce(&local_sum, &global_sum, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    end_time = MPI_Wtime();
    total_time = end_time - start_time;

    if (rank == 0)
    {
        FILE* f = fopen("mpi_sum_time.txt", "a");
        if (f == NULL) {
            fprintf(stderr, "Error: Cannot open output file\n");
            free(array);
            return 1;
        }
        fprintf(f, "%lf\n", total_time);
        fclose(f);
    }

    if (rank == 0)
    {
        free(array);
        free(sendcounts);
        free(displs);
    }
    free(local_array);

    MPI_Finalize();
    return 0;
}#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank, size;
    int array_size = 0;
    int* array = NULL;
    int local_size = 0;
    int* local_array = NULL;
    long long global_sum = 0, local_sum = 0;
    double start_time, end_time, total_time;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);


    if (argc != 2) {
        if (rank == 0) {
            printf("Usage: %s <array_size>\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }

    array_size = atoi(argv[1]);
    if (array_size <= 100000)
    {
        if (rank == 0) {
            printf(stderr, "Error: the size of the array must be larger 100000\n");
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
        return 1;
    }

    if (rank == 0) {
        array = (int*)malloc(array_size * sizeof(int));
        if (array == NULL) {
            printf("Memmory allocation failed\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        srand(time(NULL));
        for (int i = 0; i < array_size; i++)
        {
            array[i] = rand() % 100;
        }
    }

    start_time = MPI_Wtime();

    MPI_Bcast(&array_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    local_size = array_size / size;
    if (array_size % size != 0 && rank == size - 1)
    {
        local_size += array_size % size;
    }

    local_array = (int *)malloc(local_size * sizeof(int));
    if (local_array == NULL)
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

    MPI_Scatterv(array, sendcounts, displs, MPI_INT,
                 local_array, local_size, MPI_INT,
                 0, MPI_COMM_WORLD);


    local_sum = 0;
    for (int i = 0; i < local_size; i++)
    {
        local_sum += local_array[i];
    }

    MPI_Reduce(&local_sum, &global_sum, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    end_time = MPI_Wtime();
    total_time = end_time - start_time;

    if (rank == 0)
    {
        FILE* f = fopen("mpi_sum_time.txt", "a");
        if (f == NULL) {
            fprintf(stderr, "Error: Cannot open output file\n");
            free(array);
            return 1;
        }
        fprintf(f, "%lf\n", total_time);
        fclose(f);
    }

    if (rank == 0)
    {
        free(array);
        free(sendcounts);
        free(displs);
    }
    free(local_array);

    MPI_Finalize();
    return 0;
}
