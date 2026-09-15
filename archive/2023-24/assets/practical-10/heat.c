#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//size of plate
#define M 200
#define N 1000

int main(int argc, char *argv[]) {
    printf("Heated Plate calculation\n");

    // arrays for recording temperatures
    double* u = (double*) malloc(sizeof(double) * M * N);
    double* w = (double*) malloc(sizeof(double) * M * N);

    double epsilon = 0.00001;
    int iterations;
    int iterations_print;
    double mean;

    printf("  Spatial grid of %d by %d points.\n", M, N);
    printf("  The iteration will be repeated until the change is <= %lf\n", epsilon);

    // Set the boundary values, which don't change.
    mean = 0.0;

    for (int i = 1; i < M-1; i++) {
        w[i * N + 0] = 100.0;
        w[i * N + (N-1)] = 100.0;
    }
    for (int j = 0; j < N; j++) {
        w[(M-1) * N + j] = 100.0;
        w[(0) * N + j] = 0.0;
    }

    // Average the boundary values, to come up with a reasonable initial value for the interior.
    for (int i = 1; i < M-1; i++) {
        mean += w[i * N + 0] + w[i * N + (N-1)];
    }
    for (int j = 0; j < N; j++) {
        mean += w[(M-1) * N + j] + w[0 * N + j];
    }

    mean = mean / (double) ( 2 * M + 2 * N - 4 );
    printf("\n MEAN = %lf\n", mean);

    // Initialize the interior solution to the mean value.
    for (int i = 1; i < M - 1; i++) {
        for (int j = 1; j < N - 1; j++) {
            w[i * N + j] = mean;
        }
    }

    // iterate until the new solution W differs from the old solution U by no more than EPSILON.
    iterations = 0;
    iterations_print = 1000; // print an update every 1000 iterations

    double diff = epsilon;

    while (epsilon <= diff) {
        // Save the old solution in U.
        diff = 0.0; // Some threads start their work before the value is fully reset, causing some
                    // undefined behaviour? So just do this at the very start, rather than later

        for (int i = 0; i < M; i++) {
            for (int j = 0; j < N; j++) {
                u[i * N + j] = w[i * N + j];
            }
        }

        // Determine the new estimate of the solution at the interior points.
        // The new solution W is the average of north, south, east and west neighbors.
        for (int i = 1; i < M - 1; i++) {
            for (int j = 1; j < N - 1; j++) {
                w[i * N + j] = (u[(i-1) * N + j] + u[(i+1) * N + j] + u[i * N + (j-1)] + u[i * N + (j+1)]) / 4.0;
            }
        }

        // Find the largest difference between the old and new values
        for (int i = 1; i < M - 1; i++) {
            for (int j = 1; j < N - 1; j++) {
                double d = fabs(w[i * N + j]-u[i * N + j]);
                diff = (d > diff) ? d : diff;
            }
        }

        iterations++;
        if (iterations % iterations_print == 0) {
            printf("  %8d  %f\n", iterations, diff);
        }
    }

    printf("\n  %8d  %f\n", iterations, diff);

    printf("\n  End of execution.\n");

    // write output to a csv file
    FILE *output = fopen("./output.csv", "w");
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N-1; j++) {
            fprintf(output, "%lf,", w[i * N + j]);
        }
        fprintf(output, "%lf\n", w[i * N + (N-1)]);
    }
    fclose(output);
}
