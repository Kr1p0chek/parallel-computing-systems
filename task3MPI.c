#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

// Function prototypes
void addArrays(double *arr1, double *arr2, double *result, int size);
void subtractArrays(double *arr1, double *arr2, double *result, int size);
void multiplyArrays(double *arr1, double *arr2, double *result, int size);
void divideArrays(double *arr1, double *arr2, double *result, int size);

int main(int argc, char **argv)
{
    int rank, size, array_size = 0, local_size = 0;
    double *arr1 = NULL, *arr2 = NULL, *local_arr1 = NULL, *local_arr2 = NULL, *local_result_add = NULL, *local_result_sub = NULL,
           *local_result_mul = NULL, *local_result_div = NULL, *result_add = NULL, *result_sub = NULL, *result_mul = NULL,
           *result_div = NULL;
    FILE *fp = NULL;
    double start_time, end_time, total_time;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0)
    {
        fp = fopen("array_size.txt", "r");
        if (fp == NULL)
        {
            fprintf(stderr, "Error: Could not open file array_size.txt\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        if (fscanf(fp, "%d", &array_size) != 1)
        {
            fprintf(stderr, "Error: Could not read array size from file\n");
            fclose(fp);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        fclose(fp);

        if (array_size <= 100)
        {
            fprintf(stderr, "Error: Array size must be greater than 100000\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        arr1 = (double *)malloc(array_size * sizeof(double));
        arr2 = (double *)malloc(array_size * sizeof(double));
        result_add = (double *)malloc(array_size * sizeof(double));
        result_sub = (double *)malloc(array_size * sizeof(double));
        result_mul = (double *)malloc(array_size * sizeof(double));
        result_div = (double *)malloc(array_size * sizeof(double));

        if (!arr1 || !arr2 || !result_add || !result_sub || !result_mul || !result_div)
        {
            fprintf(stderr, "Error: Memory allocation failed on process 0\n");
            if (arr1)
                free(arr1);
            if (arr2)
                free(arr2);
            if (result_add)
                free(result_add);
            if (result_sub)
                free(result_sub);
            if (result_mul)
                free(result_mul);
            if (result_div)
                free(result_div);

            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        srand(time(NULL));
        for (int i = 0; i < array_size; i++)
        {
            arr1[i] = (double)rand() / RAND_MAX * 10.0;
            arr2[i] = (double)rand() / RAND_MAX * 10.0;
        }
    }

    MPI_Bcast(&array_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    local_size = array_size / size;
    if (array_size % size != 0 && rank == size - 1)
    {
        local_size += array_size % size;
    }

    local_arr1 = (double *)malloc(local_size * sizeof(double));
    local_arr2 = (double *)malloc(local_size * sizeof(double));
    local_result_add = (double *)malloc(local_size * sizeof(double));
    local_result_sub = (double *)malloc(local_size * sizeof(double));
    local_result_mul = (double *)malloc(local_size * sizeof(double));
    local_result_div = (double *)malloc(local_size * sizeof(double));

    if (!local_arr1 || !local_arr2 || !local_result_add || !local_result_sub || !local_result_mul || !local_result_div)
    {
        fprintf(stderr, "Error: Memory allocation failed on process %d\n", rank);

        if (local_arr1)
            free(local_arr1);
        if (local_arr2)
            free(local_arr2);
        if (local_result_add)
            free(local_result_add);
        if (local_result_sub)
            free(local_result_sub);
        if (local_result_mul)
            free(local_result_mul);
        if (local_result_div)
            free(local_result_div);

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

    MPI_Scatterv(arr1, sendcounts, displs, MPI_DOUBLE, local_arr1, local_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Scatterv(arr2, sendcounts, displs, MPI_DOUBLE, local_arr2, local_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    start_time = MPI_Wtime();

    addArrays(local_arr1, local_arr2, local_result_add, local_size);
    subtractArrays(local_arr1, local_arr2, local_result_sub, local_size);
    multiplyArrays(local_arr1, local_arr2, local_result_mul, local_size);
    divideArrays(local_arr1, local_arr2, local_result_div, local_size);

    end_time = MPI_Wtime();
    total_time = end_time - start_time;

    MPI_Gatherv(local_result_add, local_size, MPI_DOUBLE, result_add, sendcounts, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Gatherv(local_result_sub, local_size, MPI_DOUBLE, result_sub, sendcounts, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Gatherv(local_result_mul, local_size, MPI_DOUBLE, result_mul, sendcounts, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Gatherv(local_result_div, local_size, MPI_DOUBLE, result_div, sendcounts, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        printf("Parallel execution time with MPI: %.6f seconds\n", total_time);

        /* Optional:  Print some elements of the resulting arrays for verification
         printf("Result of addition (first 10 elements): ");
         for (int i = 0; i < 10 && i < array_size; i++) {
             printf("%.2f ", result_add[i]);
         }
         printf("\n");
         */
    }

    if (rank == 0)
    {
        free(arr1);
        free(arr2);
        free(result_add);
        free(result_sub);
        free(result_mul);
        free(result_div);
        free(sendcounts);
        free(displs);
    }
    free(local_arr1);
    free(local_arr2);
    free(local_result_add);
    free(local_result_sub);
    free(local_result_mul);
    free(local_result_div);

    MPI_Finalize();
    return 0;
}

void addArrays(double *arr1, double *arr2, double *result, int size)
{
    for (int i = 0; i < size; i++)
    {
        result[i] = arr1[i] + arr2[i];
    }
}

void subtractArrays(double *arr1, double *arr2, double *result, int size)
{
    for (int i = 0; i < size; i++)
    {
        result[i] = arr1[i] - arr2[i];
    }
}

void multiplyArrays(double *arr1, double *arr2, double *result, int size)
{
    for (int i = 0; i < size; i++)
    {
        result[i] = arr1[i] * arr2[i];
    }
}

void divideArrays(double *arr1, double *arr2, double *result, int size)
{
    for (int i = 0; i < size; i++)
    {
        if (arr2[i] == 0.0)
        {
            result[i] = 0.0;
            fprintf(stderr, "Warning: Division by zero\n");
        }
        else
        {
            result[i] = arr1[i] / arr2[i];
        }
    }
}