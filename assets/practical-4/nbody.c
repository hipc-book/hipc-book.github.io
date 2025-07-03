#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <immintrin.h>

#include <sys/time.h>
#include <sys/times.h>
#include <sys/resource.h>

// Simulation parameters.
float eps = 0.00125f;
float dmp = 0.995f;
float dt = 0.001f;

/**
 * Main function.
 */

int main(int argc, char *argv[]) {
	int N = atoi(argv[1]);
	int steps = atoi(argv[2]);

	// Allocate arrays, aligned to 16 bytes.
	float *x = (float *) malloc(N * sizeof(float));
	float *y = (float *) malloc(N * sizeof(float));
	float *z = (float *) malloc(N * sizeof(float));
	float *vx = (float *) malloc(N * sizeof(float));
	float *vy = (float *) malloc(N * sizeof(float));
	float *vz = (float *) malloc(N * sizeof(float));
	float *m = (float *) malloc(N * sizeof(float));

	// Initialise array contents.
	srand(42);
	for (int i = 0; i < N; i++)	{
		x[i] = ((rand() / (float)RAND_MAX) * 2 - 1);
		y[i] = ((rand() / (float)RAND_MAX) * 2 - 1);
		z[i] = ((rand() / (float)RAND_MAX) * 2 - 1);
		vx[i] = ((rand() / (float)RAND_MAX) * 2 - 1);
		vy[i] = ((rand() / (float)RAND_MAX) * 2 - 1);
		vz[i] = ((rand() / (float)RAND_MAX) * 2 - 1);
		m[i] = (rand() / (float)RAND_MAX);
	}

	// Current timestep.
	int t = 0;

	while (t < steps) {
		for (int i = 0; i < N; i++) {
			// start with zero acceleration for each particle
			float ax = 0.0f;
			float ay = 0.0f;
			float az = 0.0f;

			// calculate the acceleration on particle i from all other particles
			for (int j = 0; j < N; j++) {
				float rx = x[j] - x[i];
				float ry = y[j] - y[i];
				float rz = z[j] - z[i];

				float r2 = rx * rx + ry * ry + rz * rz + eps;
				
				float r2inv = 1.0f / sqrt(r2);
				
				float r6inv = r2inv * r2inv * r2inv;
				
				float s = m[j] * r6inv;
				
				ax += s * rx;
				ay += s * ry;
				az += s * rz;
			}

			// update the velocity based on calculated acceperation
			vx[i] += dmp * (dt * ax);
			vy[i] += dmp * (dt * ay);
			vz[i] += dmp * (dt * az);
		}

		// update the position of each particle
		// reflect boundary particles
		for (int i = 0; i < N; i++)	{
			x[i] += dt * vx[i];
			y[i] += dt * vy[i];
			z[i] += dt * vz[i];
			if (x[i] >= 1.0f || x[i] <= -1.0f) vx[i] *= -1.0f;
			if (y[i] >= 1.0f || y[i] <= -1.0f) vy[i] *= -1.0f;
			if (z[i] >= 1.0f || z[i] <= -1.0f) vz[i] *= -1.0f;
		}
		t++;
	}

    float phi = 0.0f;
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			float rx = x[j] - x[i];
			float ry = y[j] - y[i];
			float rz = z[j] - z[i];
			float r2 = rx*rx + ry*ry + rz*rz + eps;
			float r2inv = 1.0 / sqrt(r2);
			float r6inv = r2inv * r2inv * r2inv;
			phi += m[j] * r6inv;
		}
	}

	printf(" Answer = %f\n", phi);

	// Tidy up.
	free(x);
	free(y);
	free(z);
	free(vx);
	free(vy);
	free(vz);
	free(m);
}
