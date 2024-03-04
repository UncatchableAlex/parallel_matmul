#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include "mmm.h"
#define MAX(x, y) (x > y ? x : y)
#define MIN(x, y) (x < y ? x : y)

/**
 * Allocate and initialize the matrices on the heap. Populate
 * the input matrices with random numbers from 0 to 99
 */
void mmm_init() {
	A = (double **) malloc(size * sizeof(double*));
	B = (double **) malloc(size * sizeof(double*));
	SEQ_MATRIX = (double **) malloc(size * sizeof(double*));
	PAR_MATRIX = (double **) malloc(size * sizeof(double*));
	
	// allocate the rest of the matrices
	for (int i = 0; i < size; i++) {
		A[i] = (double*) malloc(size * sizeof(double));
		B[i] = (double*) malloc(size * sizeof(double));
		SEQ_MATRIX[i] = (double*) malloc(size * sizeof(double));
		PAR_MATRIX[i] = (double*) malloc(size * sizeof(double));
		for (int j = 0; j < size; j++) {
			// fill A and B with random doubles
			A[i][j] = (((double) rand()) / RAND_MAX) * 99;
			B[i][j] = (((double) rand()) / RAND_MAX) * 99;
			// fill SEQ_MATRIX and PAR_MATRIX with zeros:
			SEQ_MATRIX[i][j] = 0.0;
			PAR_MATRIX[i][j] = 0.0;
		}
	}
	srand((unsigned)time(NULL));	// seed the random number generator
}

/**
 * Reset a given matrix to zeroes (their size is in the global var)
 * @param matrix pointer to a 2D array
 */
void mmm_reset(double **matrix) {
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			matrix[i][j] = 0;
		}
	}
}

/**
 * Free up memory allocated to all matrices
 * (their size is in the global var)
 */
void mmm_freeup() {
	// free each row
	for (int i = 0; i < size; i++) {
		free(A[i]);
		A[i] = NULL;  // dangling pointer
		free(B[i]);
		B[i] = NULL;  // dangling pointer
		free(SEQ_MATRIX[i]);
		SEQ_MATRIX[i] = NULL;
		free(PAR_MATRIX[i]);
		PAR_MATRIX[i] = NULL;
	}
	// free original array
	free(A);
	free(B);
	free(SEQ_MATRIX);
	free(PAR_MATRIX);
	A = NULL;  // dangling pointers
	B = NULL;
	SEQ_MATRIX = NULL;
	PAR_MATRIX = NULL;
}

/**
 * Sequential MMM (size is in the global var)
 */
void mmm_seq() {
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			double total = 0;
			for (int k = 0; k < size; k++) {
				total += A[i][k] * B[k][j];
			}
			SEQ_MATRIX[i][j] = total;
		}
	}
}

/**
 * Find a bunch of dot products to fill in a section of the PAR_MATRIX.
 * 
 * All dot products in PAR_MATRIX are assigned a number between 0 and n^2 - 1 where
 * the 0th dot product is the one in the top left corner, and the n^2 -1st dot product is the one in the bottom right.
 * Their indexes increase going left to right and top to bottom (like the order in which one might read words on a page).
 * 
 * This function will fill in a bunch of the dot products starting at the given "first" dot product.
*/
void *mmm_par_subtask(void *arg) {
	// this is the first dot product that this thread will perform
	int first = *((int*)arg);
	int dotsPerThread = (size*size/num_threads) + 1;
	// the first dot product that this thread WONT perform:
	int last = first + dotsPerThread;
	// the row of the first dot product
	int firstRow = first/size;
	// the column of the first dot product:
	int firstCol = first%size;
	// the column of the last dot product:
	int lastCol = last%size;
	// the row of the last dot product
	int lastRow = last/size;
	// if last is too large, make sure that we don't write out of bounds:
	if (lastRow > size - 1) {
		lastRow = size - 1;
		lastCol = size;
	}
	printf("start  %d    end %d    firstRow %d   lastRow %d   firstCol %d    lastCol %d\n", first, last, firstRow, lastRow, firstCol, lastCol);
	for (int i = firstRow; i <= lastRow; i++) {
		for (int j = ((i == firstRow) ? firstCol : 0); j < ((i == lastRow) ? lastCol : size); j++) {
			//printf("i: %d   j: %d\n", i,j);
			// do the dot product for row i col j of PAR_MATRIX
			double total = 0;
			for (int k = 0; k < size; k++) {
				total += (A[i][k] * B[k][j]);
			}
			PAR_MATRIX[i][j] = total;
		}
	}
	return NULL;

}

/**
 * Parallel MMM
 */
void mmm_par() {
	// find out how many dot products each thread will have to perform
	int dotsPerThread = (size*size/num_threads) + 1;
	pthread_t tid[num_threads];
	// allocate memory for our threads' starting positions (the first dot product that each will perform)
	int *startingPositions = (int*) malloc(sizeof(int)*(num_threads));
	// start the first thread
	startingPositions[0] = 0;
	pthread_create(&tid[0], NULL, mmm_par_subtask, startingPositions);
	// start the other threads:
	for (int i = 1; i < num_threads; i++) {
		// set each thread to work calculating its first dot product
		startingPositions[i] = startingPositions[i - 1] + dotsPerThread; 
		pthread_create(&tid[i], NULL, mmm_par_subtask, startingPositions + i);
	}
	// reap all threads
	for (int i = 0; i < num_threads; i++) {
		pthread_join(tid[i], NULL);
	}
	// free our memory
	free(startingPositions);
	startingPositions = NULL;
}

/**
 * Verifies the correctness between the matrices generated by
 * the sequential run and the parallel run.
 *
 * @return the largest error between two corresponding elements
 * in the result matrices
 */
double mmm_verify() {
	double res = 0;
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			res = MAX(fabs(PAR_MATRIX[i][j] - SEQ_MATRIX[i][j]), res);
		}
	}
	return res;
}
