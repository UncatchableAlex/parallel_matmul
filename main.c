#include <stdio.h>
#include <stdlib.h>
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
	if (argc != 4 || (num_threads = strtol(argv[2], &ptr, 10)) <= 0 || (size = strtol(argv[3], &ptr, 10)) <= 0) {
		printf("Usage: ./mmmSol S <size>\nUsage: ./mmmSol P <num threads> <size>\n");
		return 0;
	}
	mode = argv[1][0];
	if (argv[1][0] != 'S' && argv[1][0] != 'P') {
		printf("Error: mode must be either S (sequential) or P (parallel) received %c\n", argv[1][0]);
		return 0;
	}

	// initialize my matrices
	mmm_init();

	double clockstart, clockend;
	clockstart = rtclock();	// start the clock

	if (mode == 'S') {
		mmm_seq();
	}
	else {
		mmm_par();
	}
	
	clockend = rtclock(); // stop the clock
	printf("Time taken: %.6f sec\n", (clockend - clockstart));

	if (mode == 'P') {
		mmm_seq();
		double error = mmm_verify();
		printf("Error between parallel and sequential matrix multiplication: %lf\n", error);
	}
	// free some stuff up
	mmm_freeup();
	return 0;
}
