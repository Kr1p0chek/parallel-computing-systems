#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void swap(int* arr, int i1, int i2) {
    int temp = arr[i1];
    arr[i1] = arr[i2];
    arr[i2] = temp;
}

void bubble_sort(int* arr, int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                swap(arr, j, j + 1);
            }
        }
    }
}

int is_sorted(int* arr, int n) {
    for (int i = 0; i < n - 1; i++) {
        if (arr[i] > arr[i + 1]) return 0;
    }
    return 1;
}

int* merge(int* arr1, int size1, int* arr2, int size2) {
    int first = 0;
    int second = 0;
    int sort_index = 0;
    int* sorted;
    MPI_Alloc_mem(sizeof(int) * (size1 + size2), MPI_INFO_NULL, &sorted);
    while (first < size1 && second < size2) {
        if (arr1[first] < arr2[second]) {
            sorted[sort_index++] = arr1[first++];
        }
        else {
            sorted[sort_index++] = arr2[second++];
        }
    }
    while (first < size1) {
        sorted[sort_index++] = arr1[first++];
    }

    while (second < size2) {
        sorted[sort_index++] = arr2[second++];
    }

    return sorted;
}

int next_gap(int gap) {
    if (gap <= 1) {
        return 0;
    }
    return (gap / 2) + (gap % 2);
}

void compare_split(int* self_arr, int size, int self_id, int rank1, int rank2) {
    int* other_arr = (int*)malloc(size * sizeof(int));
    MPI_Status status;
    MPI_Sendrecv(
        self_arr,
        size,
        MPI_INT,
        ((self_id == rank1) ? rank2 : rank1),
        0,
        other_arr,
        size,
        MPI_INT,
        ((self_id == rank1) ? rank2 : rank1),
        0,
        MPI_COMM_WORLD,
        &status
    );

    int i, j, gap = size + size;

    if (self_id == rank1) {
        for (gap = next_gap(gap); gap > 0; gap = next_gap(gap)) {
            for (i = 0; i + gap < size; i++) {
                if (self_arr[i] > self_arr[i + gap]) {
                    swap(self_arr, i, i + gap);
                }
            }

            for (j = gap > size ? gap - size : 0; i < size && j < size; i++, j++) {
                if (self_arr[i] > other_arr[j]) {
                    int temp = self_arr[i];
                    self_arr[i] = other_arr[j];
                    other_arr[j] = temp;
                }
            }

            if (self_id == rank2 && j < size) {
                for (j = 0; j + gap < size; j++) {
                    if (other_arr[j] > other_arr[j + gap]) {
                        swap(other_arr, j, j + gap);
                    }
                }
            }
        }
    }
    else {
        for (gap = next_gap(gap); gap > 0; gap = next_gap(gap)) {
            for (i = 0; i + gap < size; i++) {
                if (other_arr[i] > other_arr[i + gap]) {
                    swap(other_arr, i, i + gap);
                }
            }

            for (j = gap > size ? gap - size : 0; i < size && j < size; i++, j++) {
                if (other_arr[i] > self_arr[j]) {
                    int temp = other_arr[i];
                    other_arr[i] = self_arr[j];
                    self_arr[j] = temp;
                }
            }

            if (self_id == rank2 && j < size) {
                for (j = 0; j + gap < size; j++) {
                    if (self_arr[j] > self_arr[j + gap]) {
                        swap(self_arr, j, j + gap);
                    }
                }
            }
        }
    }
    free(other_arr);
}

int main(int argc, char** argv) {
    int num_proc;
    int id;
    int size;
    int chunk_size;
    int* arr = NULL;
    int* chunk;
    double time_taken;

    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    if (argc != 2) {
        if (id == 0) {
            printf("Usage: %s <array_size>\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }

    size = atoi(argv[1]);
    if (size <= 1000) {
        if (id == 0) {
            fprintf(stderr, "Error: the size of the array must be larger than 1000\n");
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
        return 1;
    }

    // Calculate chunk size
    chunk_size = size / num_proc;
    if (size % num_proc != 0) {
        chunk_size++; // Handle remainder
    }

    if (id == 0) {
        arr = (int*)malloc(size * sizeof(int));
        if (arr == NULL) {
            printf("Memory allocation failed\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        srand(time(NULL));
        for (int i = 0; i < size; i++) {
            arr[i] = rand() % 100;
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);
    time_taken = MPI_Wtime();

    // Broadcast the chunk size to all processes
    MPI_Bcast(&chunk_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Allocate memory for the chunk
    chunk = (int*)malloc(chunk_size * sizeof(int));
    if (chunk == NULL) {
        printf("Process %d: Memory allocation failed\n", id);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Scatter the data
    MPI_Scatter(arr, chunk_size, MPI_INT, chunk, chunk_size, MPI_INT, 0, MPI_COMM_WORLD);

    // Free the original array on root
    if (id == 0) {
        free(arr);
    }

    // Sort the local chunk
    bubble_sort(chunk, chunk_size);

    MPI_Barrier(MPI_COMM_WORLD);

    // Odd-even transposition sort
    for (int i = 0; i < num_proc; i++) {
        if (i % 2 == 0) { // Even phase
            if ((id % 2 == 0) && (id < num_proc - 1)) {
                compare_split(chunk, chunk_size, id, id, id + 1);
            }
            else if (id % 2 == 1) {
                compare_split(chunk, chunk_size, id, id - 1, id);
            }
        }
        else { // Odd phase
            if ((id % 2 == 1) && (id < num_proc - 1)) {
                compare_split(chunk, chunk_size, id, id, id + 1);
            }
            else if ((id % 2 == 0) && (id > 0)) {
                compare_split(chunk, chunk_size, id, id - 1, id);
            }
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }
    // Gather the results
    if (id == 0) {
        arr = (int*)malloc(size * sizeof(int));
    }

    MPI_Gather(chunk, chunk_size, MPI_INT, arr, chunk_size, MPI_INT, 0, MPI_COMM_WORLD);

    time_taken = MPI_Wtime() - time_taken;

    free(chunk);

    if (id == 0) {
        if (!is_sorted(arr, size)) {
            printf("Sort error");
            return 1;
        }
        FILE* f = fopen("mpi_sort_time.txt", "a");
        fprintf(f, "%lf\n", time_taken);
        fclose(f);
    }

    MPI_Finalize();
    return 0;
}
