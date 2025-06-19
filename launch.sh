#!/bin/bash
#
#BSUB -J MyMPIJob
#BSUB -W 00:20
#BSUB -n 2
#BSUB -R "span[ptile=2]"
#BSUB -oo mpi2log.out
#BSUB -eo mpi2err.err

module load mpi/openmpi-x86_64
iterations=100

> mpi_sort_time.txt

for ((i=1; i<=$iterations; i++)); do
    mpirun ./task2mpi 1000000
done

awk '{
    sum+=$1
    if (NR==1) {min=$1; max=$1}
    else {if ($1<min) min=$1; if ($1>max) max=$1}
} END {
    printf "Общее время выполнения: %.3f сек\nСреднее время: %.3f сек\nМинимум: %.3f сек\nМаксимум: %.3f сек\n",
    sum, sum/NR, min, max
}' mpi_sort_time.txt
