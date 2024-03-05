#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rtclock.h"
#include "mmm.h"

// shared  globals
unsigned int mode;
unsigned int size, num_threads;
double **A, **B, **SEQ_MATRIX, **PAR_MATRIX;

int main(int argc, char *argv[]) {
	// if there are not 3 arguments supplied, or an invalid number was given for the number of threads or size of matrices, 
	// print a usage error
	char *ptr;
	int validInput = 0;
	if (argc >= 2 &&  strlen(argv[1]) == 1) {
		mode = argv[1][0];
	}
	// ensure that the mode is valid:
	if (mode != 'S' && mode != 'P') {
		printf("Error: mode must be either S (sequential) or P (parallel).\n");
		return 0;
	}
	// check if the input is formatted correctly for parallel computation:
	if (argc == 4 && mode == 'P') {
		size = strtol(argv[3], &ptr, 10);
		num_threads = strtol(argv[2], &ptr, 10);
		if (size > 0 && num_threads > 0) {
			validInput = 1;
		}
	}
	// check if the input is formatted correctly for sequential computation
	if (argc == 3 && mode == 'S') {
		size = strtol(argv[2], &ptr, 10);
		if (size > 0) {
			validInput = 1;
		}
	}
	// if the input is NOT formatted properly, then print a message and return
	if (!validInput) {
		printf("Usage: ./mmmSol S <size>\nUsage: ./mmmSol P <num threads> <size>\n");
		return 0;
	}

	// initialize my matrices
	mmm_init();

	// print the intialization:
	printf("========\nmode: %s\nthread count: %d\nsize: %d\n========\n", mode == 'S' ? "sequential" : "parallel", num_threads, size);
	// track how much time it takes for the sequential runs
	double seqTime = 0;
	// set how many times to multiply the matrices in each selected mode:
	int runs = 3;
	for (int i = 0; i <= runs; i++) {
		double clockstart, clockend;
		clockstart = rtclock();	// start the clock
		mmm_seq();
		clockend = rtclock(); // stop the clock
		// the first loop doesn't count as a run:
		seqTime += !i ? 0 : (clockend - clockstart);
	}
	printf("Sequential Time (avg of 3 runs): %.6f sec\n", seqTime/runs);
	// if we want to do the parallel calculation as well, then continue on:
	if (mode == 'P') {
		double parTime = 0;
		for (int i = 0; i <= runs; i++) {
			double clockstart, clockend;
			clockstart = rtclock();	// start the clock
			mmm_par();
			clockend = rtclock(); // stop the clock
			// the first loop doesn't count as a run:
			parTime += !i ? 0 : (clockend - clockstart);
		}
		printf("Parallel Time (avg of %d runs): %.6f sec\nSpeedup: %.6f\n", runs, parTime/runs, seqTime/parTime);
		// verify our results:
		printf("Verifying... largest error between parallel and sequential matrix: %lf\n", mmm_verify());
	}
	// free some stuff up
	mmm_freeup();
	return 0;
}
