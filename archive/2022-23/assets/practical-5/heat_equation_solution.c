#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

//size of plate
#define M 200
#define N 1000

double **alloc_2d_array(int m, int n) {
    double **x;
    int i;

    x = (double **)malloc(m*sizeof(double *));
    x[0] = (double *)calloc(m*n,sizeof(double));
    for ( i = 1; i < m; i++ )
        x[i] = &x[0][i*n];
    return x;
}

void free_2d_array(double ** array) {
    free(array[0]);
    free(array);
}

int main(int argc, char *argv[]) {
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) printf("Heated Plate calculation\n");

    // arrays for recording temperatures

    // calculate the start and end indicies for parallel processors
    int startj = rank * (N/size) - 1;
    if (startj < 0) startj = 0;
    int endj = (rank+1) * (N/size);
    if (endj >= N-1) endj = N - 1;

    // calculated the size of the array on each parallel processor
    int sizej = endj - startj + 1;

    // This can be used to check that the indicies and array sizes are correct
    //printf("Rank %d start and end: %d, %d, requires (M x N): (%d x %d)\n", rank, startj, endj, M, sizej);

    double** u = alloc_2d_array(M, sizej);
    double** w = alloc_2d_array(M, sizej);

    double diff;
    double epsilon = 0.00001;
    int iterations;
    int iterations_print;
    double mean;

    // set up a custom data type that is M doubles in a column. 
    // The gap will be "sizej" (i.e. the size of a row on each processor)
    MPI_Datatype my_column;
    MPI_Type_vector(M, 1, sizej, MPI_DOUBLE, &my_column);
    MPI_Type_commit(&my_column);

    if (rank == 0) printf("  Spatial grid of %d by %d points.\n", M, N);
    if (rank == 0) printf("  The iteration will be repeated until the change is <= %lf\n", epsilon);

    // Set the boundary values, which don't change.
    mean = 0.0;

    for (int i = 1; i < M-1; i++) {
        if (rank == 0) w[i][0] = 100.0; // if I'm the left most processor, set up my boundary
        if (rank == size-1) w[i][sizej-1] = 100.0; // if I'm the right most processor, set up my boundary
    }
    for (int j = 0; j < sizej; j++) {
        w[M-1][j] = 100.0;
        w[0][j] = 0.0;
    }

    // Average the boundary values, to come up with a reasonable initial value for the interior.
    for (int i = 1; i < M-1; i++) {
        if (rank == 0) mean += w[i][0]; // if I'm the left most, include the boundary in my mean
        if (rank == size-1) mean += w[i][sizej-1]; // if I'm the right most, include the boundary in my mean
    }

    // these two calculations work out where to start counting from for the average calculation
    // you shouldn't include ghost cells, so on everything but rank 0 and rank N-1, you should start the
    // calculation on index 1, and go to index sizej - 1
    int mystart = (rank == 0) ? 0 : 1;
    int myend = (rank == size-1) ? sizej : sizej-1;
    for (int j = mystart; j < myend; j++) { // remove the ghost cells
        mean += w[M-1][j] + w[0][j];
    }

    // perform a sum reduction to help calculate the global mean value
    MPI_Allreduce(MPI_IN_PLACE, &mean, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

    mean = mean / (double) (2 * M + 2 * N - 4);
    if (rank == 0) printf("\n MEAN = %lf\n", mean);

    // Initialize the interior solution to the mean value.
    for (int i = 1; i < M - 1; i++) {
        for (int j = 1; j < sizej-1; j++) {
            w[i][j] = mean;
        }
    }

    // iterate until the new solution W differs from the old solution U by no more than EPSILON.
    iterations = 0;
    iterations_print = 1000; // print an update every 1000 iterations

    diff = epsilon;

    while (epsilon <= diff) {
        //communicate

        // calculate process rank to left and right (using MPI_PROC_NULL if first or last process)
        int left = (rank - 1) < 0 ? MPI_PROC_NULL : rank - 1;
        int right = (rank + 1) >= size ? MPI_PROC_NULL : rank + 1;

        // exchange column 1 with ghost column sizej-1
        MPI_Sendrecv(&(w[0][1]), 1, my_column, left, 0, &(w[0][sizej-1]), 1, my_column, right, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        // exchange column sizej-2 with ghost column 0
        MPI_Sendrecv(&(w[0][sizej-2]), 1, my_column, right, 0, &(w[0][0]), 1, my_column, left, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Save the old solution in U.
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < sizej; j++) {
                u[i][j] = w[i][j];
            }
        }

        // Determine the new estimate of the solution at the interior points.
        // The new solution W is the average of north, south, east and west neighbors.
        for (int i = 1; i < M - 1; i++) {
            for (int j = 1; j < sizej - 1; j++) {
                w[i][j] = (u[i-1][j] + u[i+1][j] + u[i][j-1] + u[i][j+1]) / 4.0;
            }
        }

        // Find the largest difference between the old and new values
        diff = 0.0;
        for (int i = 1; i < M - 1; i++) {
            for (int j = 1; j < sizej - 1; j++) {
                if (diff < fabs(w[i][j]-u[i][j])) {
                    diff = fabs(w[i][j]-u[i][j]);
                }
            }
        }

        // Find the maximum difference across all processors
        MPI_Allreduce(MPI_IN_PLACE, &diff, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);

        iterations++;
        if (iterations % iterations_print == 0) {
            if (rank == 0) printf("  %8d  %f\n", iterations, diff);
        }
    }

    if (rank == 0) printf("\n  %8d  %f\n", iterations, diff);
    if (rank == 0) printf("\n  End of execution.\n");

    // This has been commented out to disable file output
    // write output to a csv file
    /*FILE *output = fopen("./output.csv", "w");
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N-1; j++) {
            fprintf(output, "%lf,", w[i][j]);
        }
        fprintf(output, "%lf\n", w[i][N-1]);
    }
    fclose(output);*/

    free_2d_array(w);
    free_2d_array(u);

    MPI_Type_free(&my_column);
    MPI_Finalize();
}
