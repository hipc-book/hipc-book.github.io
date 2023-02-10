#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//#include <mpi.h>

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
	printf("Heated Plate calculation\n");

	// arrays for recording temperatures
	double** u = alloc_2d_array(M, N);	
	double** w = alloc_2d_array(M, N);	

	double diff;
	double epsilon = 0.00001;
	int iterations;
	int iterations_print;
	double mean;
  
	printf("  Spatial grid of %d by %d points.\n", M, N);
	printf("  The iteration will be repeated until the change is <= %lf\n", epsilon); 

    // Set the boundary values, which don't change.
	mean = 0.0;

	for (int i = 1; i < M-1; i++) {
		w[i][0] = 100.0;
        w[i][N-1] = 100.0;
	}
	for (int j = 0; j < N; j++) {
        w[M-1][j] = 100.0;
        w[0][j] = 0.0;
    }

    // Average the boundary values, to come up with a reasonable initial value for the interior. 
	for (int i = 1; i < M-1; i++) {		
        mean += w[i][0] + w[i][N-1];
	}
	for (int j = 0; j < N; j++) {
        mean += w[M-1][j] + w[0][j];
	} 

	mean = mean / (double) ( 2 * M + 2 * N - 4 );
	printf("\n MEAN = %lf\n", mean);

    // Initialize the interior solution to the mean value. 
	for (int i = 1; i < M - 1; i++) {
		for (int j = 1; j < N - 1; j++) {
    	 		w[i][j] = mean;
		}
	}

    // iterate until the new solution W differs from the old solution U by no more than EPSILON. 
	iterations = 0;
	iterations_print = 1000; // print an update every 1000 iterations

	diff = epsilon;

	while (epsilon <= diff) {
        // Save the old solution in U. 
		for (int i = 0; i < M; i++) {
     		for (int j = 0; j < N; j++) {
        		u[i][j] = w[i][j];
        	}
      	}

        // Determine the new estimate of the solution at the interior points. 
        // The new solution W is the average of north, south, east and west neighbors.
      	for (int i = 1; i < M - 1; i++) {
        	for (int j = 1; j < N - 1; j++) {
        		w[i][j] = (u[i-1][j] + u[i+1][j] + u[i][j-1] + u[i][j+1]) / 4.0;
        	}
      	}

        // Find the largest difference between the old and new values
		diff = 0.0;
		for (int i = 1; i < M - 1; i++) {
			for (int j = 1; j < N - 1; j++) {
		     	if (diff < fabs(w[i][j]-u[i][j])) {
	          		diff = fabs(w[i][j]-u[i][j]);
     			}
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
            fprintf(output, "%lf,", w[i][j]);
        }
        fprintf(output, "%lf\n", w[i][N-1]);
    }
    fclose(output);

    free_2d_array(w);
    free_2d_array(u);
}
