#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <complex.h>
#include <fftw3.h>
#include <math.h>

#define FRAME_DATA_SIZE_BYTES 32

int main(int argc, char*argv[]) {	
	FILE *file;
	char *path = "test2";
	int dopt;
	int numCarriers = 16;
	
	while ((dopt = getopt(argc,argv,"p:n:")) != EOF) {
		switch (dopt) {
			break;
			case 'p':
				path = optarg;
			break;
			case 'n':
				numCarriers = atoi(optarg);
			break;
			default:
			exit(1);
		}
	}
	
	if((file = fopen(path, "r")) == NULL) {
		fprintf(stderr, "Error opening file\n");
		return 1;
	}
	
	// -------------ONLY QPSK WORKS-------------
	//convert bytes into 2 bit symbols
	size_t nread;
	unsigned char buf[numCarriers/4 * (FRAME_DATA_SIZE_BYTES/2 + 1)];
	while((nread = fread(buf, 1, sizeof buf, file)) > 0) {
		unsigned int symstream[numCarriers * (FRAME_DATA_SIZE_BYTES/2 + 1)];
		fftw_complex* fftIn = fftw_malloc(sizeof(fftw_complex) * numCarriers * (FRAME_DATA_SIZE_BYTES/2 + 1));
		double *fftOut = (double*) malloc(sizeof(double) * numCarriers * FRAME_DATA_SIZE_BYTES);
		for(int j = 0;j < numCarriers * (FRAME_DATA_SIZE_BYTES/2 + 1);j++) {
			symstream[j] = 0;
		}		

		for(int b = 0;b < nread;b++) {
			symstream[b * 4 + 3] = buf[b] & 0x03;
			symstream[b * 4 + 2] = (buf[b] & 0x0C) >> 2;
			symstream[b * 4 + 1] = (buf[b] & 0x30) >> 4;
			symstream[b * 4] = (buf[b] & 0xC0) >> 6;
		}

		//constellation mapping
		for(int c = 0;c < numCarriers * (FRAME_DATA_SIZE_BYTES/2 + 1);c++) {
			if(c < nread * 4) {
				if(symstream[c] == 0) 
					fftIn[c] = 0.707 + I * 0.707;
				else if(symstream[c] == 1)
					fftIn[c] = 0.707 - I * 0.707;
				else if(symstream[c] == 2)
					fftIn[c] = -0.707 + I * 0.707;
				else
					fftIn[c] = -0.707 - I * 0.707;
			} else {
				fftIn[c] = 0;
			}
			printf("%lf, %lf\n", creal(fftIn[c]), cimag(fftIn[c]));
		}

		printf("--------------------------------------------\n");
		//serial to parallel + IFFT (Hermitian symmetry applied inside the function) + parallel to serial
		int size1 = numCarriers;
		int size2 = FRAME_DATA_SIZE_BYTES;
		
		fftw_plan txPlan = fftw_plan_dft_c2r_2d(size1, size2, fftIn, fftOut, FFTW_ESTIMATE);
		fftw_execute(txPlan);
		for(int c = 0;c < FRAME_DATA_SIZE_BYTES * numCarriers;c++) {
			printf("%lf\n", fftOut[c]);
		}
		printf("--------------------------------------------\n");


		fftw_destroy_plan (txPlan);
		fftw_destroy_plan (rxPlan);
		free(fftIn);
		free(fftOut);
		free(rxOut);
		exit(1);
	}
	
	fclose(file);
	return 0;
}
