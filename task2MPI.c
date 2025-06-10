#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

void swap(int *a, int *b)
{
    int t = *a;
    *a = *b;
    *b = t;
}

int main(int argc, char **argv)
{
    int rank, size;
    int array_size = 0;
    int *arr = NULL;
    int local_size = 0;
    int *local_arr = NULL;
    double start_time, end_time;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0)
    {
        FILE *fp = fopen("array_size.txt", "r");
        if (fp == NULL)
        {
            fprintf(stderr, "Error: couldn't open the file array_size.txt\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
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

        srand(time(NULL));
        for (int i = 0; i < array_size; i++)
        {
            arr[i] = rand() % 1000;
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

    for (int i = 0; i < array_size; i++)
    {
        if (i % 2 == 0)
        {
            for (int j = 0; j < local_size - 1; j++)
            {
                if (local_arr[j] > local_arr[j + 1])
                {
                    swap(&local_arr[j], &local_arr[j + 1]);
                }
            }
        }

        if (rank % 2 == 0)
        {
            if (rank < size - 1)
            {
                int send_val = local_arr[local_size - 1];
                int recv_val;
                MPI_Sendrecv(&send_val, 1, MPI_INT, rank + 1, 0,
                             &recv_val, 1, MPI_INT, rank + 1, 0,
                             MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                if (send_val > recv_val)
                {
                    local_arr[local_size - 1] = recv_val;
                }
            }
        }
        else
        {
            int recv_val;
            int send_val;

            if (rank > 0)
            {
                send_val = local_arr[0];
                MPI_Sendrecv(&send_val, 1, MPI_INT, rank - 1, 0,
                             &recv_val, 1, MPI_INT, rank - 1, 0,
                             MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                if (send_val < recv_val)
                {
                    local_arr[0] = recv_val;
                }
            }
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }

    end_time = MPI_Wtime();

    MPI_Gatherv(local_arr, local_size, MPI_INT,
                arr, sendcounts, displs, MPI_INT,
                0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        double elapsed_time = end_time - start_time;
        printf("Execution time (in parallel with MPI): %.6f sec\n", elapsed_time);

        /* Optional: Print the sorted array
        printf("Отсортированный массив:\n");
        for (int i = 0; i < array_size; i++) {
            printf("%d ", arr[i]);
        }
        printf("\n");
        */
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