/*#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

void addMatrices(double **matrix1, double **matrix2, double **result, int rows, int cols);
void subtractMatrices(double **matrix1, double **matrix2, double **result, int rows, int cols);
void multiplyMatrices(double **matrix1, double **matrix2, double **result, int rows, int cols);
void divideMatrices(double **matrix1, double **matrix2, double **result, int rows, int cols);
double **allocateMatrix(int rows, int cols);
void freeMatrix(double **matrix, int rows);

int main(int argc, char **argv)
{
    int rank, size, rows = 0, cols = 0, local_rows = 0;
    double **matrix1 = NULL, **matrix2 = NULL, **local_matrix1 = NULL, **local_matrix2 = NULL, **local_result_add = NULL,
           **local_result_sub = NULL, **local_result_mul = NULL, **local_result_div = NULL, **result_add = NULL,
           **result_sub = NULL, **result_mul = NULL, **result_div = NULL;
    FILE *fp = NULL;
    char filename[] = "array_size.txt"; // File name for matrix dimensions
    double start_time, end_time, total_time;
    int i, j; // Loop variables

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Process 0 reads matrix dimensions from file
    if (rank == 0)
    {
        fp = fopen(filename, "r");
        if (fp == NULL)
        {
            fprintf(stderr, "Error: Could not open file %s\n", filename);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        if (fscanf(fp, "%d %d", &rows, &cols) != 2)
        {
            fprintf(stderr, "Error: Could not read rows and columns from %s\n", filename);
            fclose(fp);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        fclose(fp);

        if (rows <= 0 || cols <= 0)
        {
            fprintf(stderr, "Error: Matrix dimensions must be positive\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&cols, 1, MPI_INT, 0, MPI_COMM_WORLD);

    local_rows = rows / size;
    int remainder = rows % size;
    if (rank < remainder)
    {
        local_rows++;
    }
    local_matrix1 = allocateMatrix(local_rows, cols);
    local_matrix2 = allocateMatrix(local_rows, cols);
    local_result_add = allocateMatrix(local_rows, cols);
    local_result_sub = allocateMatrix(local_rows, cols);
    local_result_mul = allocateMatrix(local_rows, cols);
    local_result_div = allocateMatrix(local_rows, cols);

    if (local_matrix1 == NULL || local_matrix2 == NULL || local_result_add == NULL || local_result_sub == NULL || local_result_mul == NULL || local_result_div == NULL)
    {
        fprintf(stderr, "Error: Memory allocation for local matrices failed on process %d\n", rank);
        if (local_matrix1)
            freeMatrix(local_matrix1, local_rows);
        if (local_matrix2)
            freeMatrix(local_matrix2, local_rows);
        if (local_result_add)
            freeMatrix(local_result_add, local_rows);
        if (local_result_sub)
            freeMatrix(local_result_sub, local_rows);
        if (local_result_mul)
            freeMatrix(local_result_mul, local_rows);
        if (local_result_div)
            freeMatrix(local_result_div, local_rows);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    if (rank == 0)
    {
        matrix1 = allocateMatrix(rows, cols);
        matrix2 = allocateMatrix(rows, cols);
        result_add = allocateMatrix(rows, cols);
        result_sub = allocateMatrix(rows, cols);
        result_mul = allocateMatrix(rows, cols);
        result_div = allocateMatrix(rows, cols);
        if (!matrix1 || !matrix2 || !result_add || !result_sub || !result_mul || !result_div)
        {
            fprintf(stderr, "Error: Memory allocation for global matrices failed on process %d\n", rank);
            if (matrix1)
                freeMatrix(matrix1, rows);
            if (matrix2)
                freeMatrix(matrix2, rows);
            if (result_add)
                freeMatrix(result_add, rows);
            if (result_sub)
                freeMatrix(result_sub, rows);
            if (result_mul)
                freeMatrix(result_mul, rows);
            if (result_div)
                freeMatrix(result_div, rows);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        srand(time(NULL));
        for (i = 0; i < rows; i++)
        {
            for (j = 0; j < cols; j++)
            {
                matrix1[i][j] = (double)rand() / RAND_MAX * 10.0;
                matrix2[i][j] = (double)rand() / RAND_MAX * 10.0;
            }
        }
    }

    int *sendcounts = (int *)malloc(size * sizeof(int));
    int *displs = (int *)malloc(size * sizeof(int));
    if (sendcounts == NULL || displs == NULL)
    {
        fprintf(stderr, "Error: Memory allocation for scatter parameters failed on process %d\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    int base_rows_per_proc = rows / size;
    int remainder_rows = rows % size;
    int row_index = 0;
    for (i = 0; i < size; i++)
    {
        sendcounts[i] = base_rows_per_proc;
        if (i < remainder_rows)
        {
            sendcounts[i]++;
        }
    }

    displs[0] = 0;
    for (i = 1; i < size; i++)
    {
        displs[i] = displs[i - 1] + sendcounts[i - 1];
    }

    MPI_Scatterv(matrix1, sendcounts, displs, MPI_DOUBLE, (void *)local_matrix1[0], local_rows * cols, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Scatterv(matrix2, sendcounts, displs, MPI_DOUBLE, (void *)local_matrix2[0], local_rows * cols, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);
    start_time = MPI_Wtime();

    addMatrices(local_matrix1, local_matrix2, local_result_add, local_rows, cols);
    subtractMatrices(local_matrix1, local_matrix2, local_result_sub, local_rows, cols);
    multiplyMatrices(local_matrix1, local_matrix2, local_result_mul, local_rows, cols);
    divideMatrices(local_matrix1, local_matrix2, local_result_div, local_rows, cols);

    end_time = MPI_Wtime();
    total_time = end_time - start_time;

    MPI_Gatherv(local_result_add[0], local_rows * cols, MPI_DOUBLE, result_add, sendcounts, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Gatherv(local_result_sub[0], local_rows * cols, MPI_DOUBLE, result_sub, sendcounts, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Gatherv(local_result_mul[0], local_rows * cols, MPI_DOUBLE, result_mul, sendcounts, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Gatherv(local_result_div[0], local_rows * cols, MPI_DOUBLE, result_div, sendcounts, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        printf("Parallel execution time with MPI: %.6f seconds\n", total_time);

        // Optional:  Print the resulting matrices (first few elements) for verification
        //printf("Result of addition (first few elements):\n");
        //for (i = 0; i < 2 && i < rows; i++) {
        //    for (j = 0; j < 2 && j < cols; j++) {
        //        printf("%.2f ", result_add[i][j]);
        //    }
        //    printf("\n");
        //}
        //
    }

    free(sendcounts);
    free(displs);
    if (rank == 0)
    {
        freeMatrix(matrix1, rows);
        freeMatrix(matrix2, rows);
        freeMatrix(result_add, rows);
        freeMatrix(result_sub, rows);
        freeMatrix(result_mul, rows);
        freeMatrix(result_div, rows);
    }
    freeMatrix(local_matrix1, local_rows);
    freeMatrix(local_matrix2, local_rows);
    freeMatrix(local_result_add, local_rows);
    freeMatrix(local_result_sub, local_rows);
    freeMatrix(local_result_mul, local_rows);
    freeMatrix(local_result_div, local_rows);

    MPI_Finalize();
    return 0;
}

void addMatrices(double **matrix1, double **matrix2, double **result, int rows, int cols)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            result[i][j] = matrix1[i][j] + matrix2[i][j];
        }
    }
}

void subtractMatrices(double **matrix1, double **matrix2, double **result, int rows, int cols)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            result[i][j] = matrix1[i][j] - matrix2[i][j];
        }
    }
}

void multiplyMatrices(double **matrix1, double **matrix2, double **result, int rows, int cols)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            result[i][j] = matrix1[i][j] * matrix2[i][j];
        }
    }
}

void divideMatrices(double **matrix1, double **matrix2, double **result, int rows, int cols)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            if (matrix2[i][j] == 0.0)
            {
                result[i][j] = 0.0; // Handle division by zero
                fprintf(stderr, "Warning: Division by zero at [%d][%d]\n", i, j);
            }
            else
            {
                result[i][j] = matrix1[i][j] / matrix2[i][j];
            }
        }
    }
}

double **allocateMatrix(int rows, int cols)
{
    double **matrix = (double **)malloc(rows * sizeof(double *));
    if (matrix == NULL)
    {
        fprintf(stderr, "Error: Could not allocate memory for rows\n");
        return NULL;
    }
    for (int i = 0; i < rows; i++)
    {
        matrix[i] = (double *)malloc(cols * sizeof(double));
        if (matrix[i] == NULL)
        {
            fprintf(stderr, "Error: Could not allocate memory for columns in row %d\n", i);
            // Free previously allocated rows to avoid memory leaks
            for (int j = 0; j < i; j++)
            {
                free(matrix[j]);
            }
            free(matrix);
            return NULL;
        }
    }
    return matrix;
}

void freeMatrix(double **matrix, int rows)
{
    for (int i = 0; i < rows; i++)
    {
        free(matrix[i]);
    }
    free(matrix);
}*/

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#include <math.h>

#define MASTER 0

void read_matrix_size_from_file(const char *filename, int *rows, int *cols)
{
    FILE *file = fopen("array_size.txt", "r");
    if (file == NULL)
    {
        perror("Error opening file");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    if (fscanf(file, "%d %d", rows, cols) != 2)
    {
        fprintf(stderr, "Error reading matrix dimensions from file\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    fclose(file);
}

void generate_random_matrix(double *matrix, int size)
{
    for (int i = 0; i < size; i++)
    {
        matrix[i] = (double)rand() / RAND_MAX * 100.0;
    }
}

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    double *matrix1 = NULL, *matrix2 = NULL;
    int rows, cols;
    double start_time, end_time;

    if (rank == MASTER)
    {
        if (argc != 2)
        {
            printf("Usage: %s <input_file>\n", argv[0]);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }

        start_time = MPI_Wtime();

        read_matrix_size_from_file(argv[1], &rows, &cols);
        printf("Matrix dimensions: %d x %d\n", rows, cols);

        srand(time(NULL));

        int total_elements = rows * cols;
        matrix1 = (double *)malloc(total_elements * sizeof(double));
        matrix2 = (double *)malloc(total_elements * sizeof(double));

        generate_random_matrix(matrix1, total_elements);
        generate_random_matrix(matrix2, total_elements);
    }

    MPI_Bcast(&rows, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
    MPI_Bcast(&cols, 1, MPI_INT, MASTER, MPI_COMM_WORLD);

    int total_elements = rows * cols;
    int elements_per_proc = total_elements / size;
    int remainder = total_elements % size;

    int *send_counts = (int *)malloc(size * sizeof(int));
    int *displs = (int *)malloc(size * sizeof(int));

    for (int i = 0; i < size; i++)
    {
        send_counts[i] = elements_per_proc + (i < remainder ? 1 : 0);
        displs[i] = (i == 0) ? 0 : (displs[i - 1] + send_counts[i - 1]);
    }

    int local_size = send_counts[rank];
    double *local_matrix1 = (double *)malloc(local_size * sizeof(double));
    double *local_matrix2 = (double *)malloc(local_size * sizeof(double));

    MPI_Scatterv(matrix1, send_counts, displs, MPI_DOUBLE, local_matrix1, local_size, MPI_DOUBLE, MASTER, MPI_COMM_WORLD);
    MPI_Scatterv(matrix2, send_counts, displs, MPI_DOUBLE, local_matrix2, local_size, MPI_DOUBLE, MASTER, MPI_COMM_WORLD);

    double *local_add = (double *)malloc(local_size * sizeof(double));
    double *local_sub = (double *)malloc(local_size * sizeof(double));
    double *local_mul = (double *)malloc(local_size * sizeof(double));
    double *local_div = (double *)malloc(local_size * sizeof(double));

    for (int i = 0; i < local_size; i++)
    {
        local_add[i] = local_matrix1[i] + local_matrix2[i];
        local_sub[i] = local_matrix1[i] - local_matrix2[i];
        local_mul[i] = local_matrix1[i] * local_matrix2[i];
        local_div[i] = (fabs(local_matrix2[i]) > 1e-10) ? local_matrix1[i] / local_matrix2[i] : INFINITY;
    }

    double *add_result = NULL, *sub_result = NULL, *mul_result = NULL, *div_result = NULL;

    if (rank == MASTER)
    {
        add_result = (double *)malloc(total_elements * sizeof(double));
        sub_result = (double *)malloc(total_elements * sizeof(double));
        mul_result = (double *)malloc(total_elements * sizeof(double));
        div_result = (double *)malloc(total_elements * sizeof(double));
    }

    MPI_Gatherv(local_add, local_size, MPI_DOUBLE, add_result, send_counts, displs, MPI_DOUBLE, MASTER, MPI_COMM_WORLD);
    MPI_Gatherv(local_sub, local_size, MPI_DOUBLE, sub_result, send_counts, displs, MPI_DOUBLE, MASTER, MPI_COMM_WORLD);
    MPI_Gatherv(local_mul, local_size, MPI_DOUBLE, mul_result, send_counts, displs, MPI_DOUBLE, MASTER, MPI_COMM_WORLD);
    MPI_Gatherv(local_div, local_size, MPI_DOUBLE, div_result, send_counts, displs, MPI_DOUBLE, MASTER, MPI_COMM_WORLD);

    if (rank == MASTER)
    {
        end_time = MPI_Wtime();
        printf("Total execution time: %.6f seconds\n", end_time - start_time);

        free(matrix1);
        free(matrix2);
        free(add_result);
        free(sub_result);
        free(mul_result);
        free(div_result);
    }

    free(local_matrix1);
    free(local_matrix2);
    free(local_add);
    free(local_sub);
    free(local_mul);
    free(local_div);
    free(send_counts);
    free(displs);

    MPI_Finalize();
    return 0;
}