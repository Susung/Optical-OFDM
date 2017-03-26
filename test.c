#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <complex.h>
#include <fftw3.h>
#include <math.h>
#include <time.h>

int main(int argc, char*argv[]) {	
	int numCarriers = 8;
	int dataLength = 32;
	
	// -------------ONLY QPSK WORKS-------------
	/* 
	 * The data length(column) for complex matrix must be (x/2 + 1) of the ifft output (real) matrix due to hermitian symmetry
	 * must be divided by 4 since every char is 1 byte
	 * -1 is added because a dummy symbol is going to be added at front and end (2 in front & 2 in the end)
	 * (fft screws up the first and last symbol for some reason)
	 */
	fftw_complex *fftIn = fftw_malloc(sizeof(fftw_complex) * numCarriers * ((dataLength/2) + 1));
	fftw_complex *fftOut = fftw_malloc(sizeof(fftw_complex) * numCarriers * ((dataLength/2) + 1));	
	double *doubleOut = (double*) malloc(sizeof(double) * numCarriers * dataLength);
	double *doubleIn = (double*) malloc(sizeof(double) * numCarriers * dataLength);
	// serial to parallel + IFFT (Hermitian symmetry applied inside the function) + parallel to serial
	// sizes must match the size of real matrix
	int size1 = numCarriers;
	int size2 = dataLength;
	fftw_plan txPlan = fftw_plan_dft_c2r_2d(size1, size2, fftIn, doubleOut, FFTW_MEASURE);
	
   time_t t;
   srand((unsigned) time(&t));
	// constellation mapping
	for(int c = 0;c < numCarriers * (dataLength/2 + 1);c++) {

		int r = rand()%4;      // returns a pseudo-random integer between 0 and RAND_MAX
		if(r == 0) 
			fftIn[c] = 0.707 + I * 0.707;
		else if(r == 1)
			fftIn[c] = 0.707 - I * 0.707;
		else if(r == 2)
			fftIn[c] = -0.707 + I * 0.707;
		else
			fftIn[c] = -0.707 - I * 0.707;
		printf("%lf, %lf\n", creal(fftIn[c]), cimag(fftIn[c]));
	}
	printf("=====================================\n");

	fftw_execute(txPlan);

	fftw_plan rxPlan = fftw_plan_dft_r2c_2d(size1, size2, doubleIn, fftOut, FFTW_MEASURE);
	
	for(int c = 0;c < numCarriers * (dataLength);c++) {
		doubleIn[c] = doubleOut[c];
	}
	
	fftw_execute(rxPlan);
	

	for(int c = 0;c < numCarriers * (dataLength/2 + 1);c++) {
		printf("%lf, %lf\n",
		creal(fftOut[c]), 
		cimag(fftOut[c]));
	}

	fftw_destroy_plan(txPlan);
	fftw_destroy_plan(rxPlan);
	free(fftIn);
	free(fftOut);
	free(doubleOut);
	free(doubleIn);
	return 0;
}
