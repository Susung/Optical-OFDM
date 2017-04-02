#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <complex.h>
#include <fftw3.h>
#include <math.h>

int main(int argc, char*argv[]) {	
	FILE *fileIn;
	FILE *fileOut;
	char *input = "test";
	char *output = "transmitted.txt";
	int dopt;
	int numCarriers = 16;
	int dataLength = 64;
	
	while ((dopt = getopt(argc,argv,"p:n:")) != EOF) {
		switch (dopt) {
			break;
			case 'p':
				input = optarg;
			break;
			case 'n':
				numCarriers = atoi(optarg);
			break;
			default:
			exit(1);
		}
	}
	
	if((fileIn = fopen(input, "r")) == NULL) {
		fprintf(stderr, "Error opening input file\n");
		return 1;
	}
	
	if((fileOut = fopen(output, "w")) == NULL) {
		fprintf(stderr, "Error opening output file\n");
		return 1;
	}
	
	// -------------ONLY QPSK WORKS-------------
	/* 
	 * The data length(column) for complex matrix must be (x/2 + 1) of the ifft output (real) matrix due to hermitian symmetry
	 * must be divided by 4 since every char is 1 byte
	 */
	int frames = 0;
	double high = 0;
	double low = 0;
	int guard = dataLength/16;
	unsigned char buf[numCarriers/4 * (dataLength/2 + 1 - 2 * guard)]; 
	size_t nread;
	while((nread = fread(buf, 1, sizeof buf, fileIn)) > 0) {
		// array to hold symbol data
		unsigned int *symstream = (unsigned int*) malloc(sizeof(unsigned int) * numCarriers * (dataLength/2 + 1 - 2 * guard)); 
		fftw_complex *fftIn = fftw_malloc(sizeof(fftw_complex) * numCarriers * (dataLength/2 + 1));
		double *fftOut = (double*) malloc(sizeof(double) * numCarriers * dataLength);
		
		// convert bytes into 4 2-bit symbols
		for(int b = 0;b < sizeof buf;b++) {
			if(b < nread) {
				symstream[b * 4 + 3] = buf[b] & 0x03;
				symstream[b * 4 + 2] = (buf[b] & 0x0C) >> 2;
				symstream[b * 4 + 1] = (buf[b] & 0x30) >> 4;
				symstream[b * 4] = (buf[b] & 0xC0) >> 6;
			} else {//zero filling
				symstream[b * 4 + 3] = 0;
				symstream[b * 4 + 2] = 0;
				symstream[b * 4 + 1] = 0;
				symstream[b * 4] = 0;
			}
			//printf("%c : ",buf[b]);
			//printf("%u %u %u %u\n", symstream[b * 4], symstream[b * 4 + 1], symstream[b * 4 + 2], symstream[b * 4 + 3]);
		}

		// constellation mapping
		for(int r = 0;r < numCarriers;r++) {
			for(int g = 0;g < guard;g++) {
				fftIn[r*(dataLength/2 + 1)+g] = 0 + I * 0;
				//printf("[%lf, %lf] ", creal(fftIn[r*(dataLength/2 + 1)+g]), cimag(fftIn[r*(dataLength/2 + 1)+g]));
			}
			for(int c = guard;c < dataLength/2 + 1 - guard;c++) {
				unsigned char sym = symstream[r*(dataLength/2 + 1 - guard*2)+(c-guard)];
				if(sym == 0) 
					fftIn[r*(dataLength/2 + 1)+c] = 0.1 + I * 0.1;
				else if(sym == 1)
					fftIn[r*(dataLength/2 + 1)+c] = 0.1 - I * 0.1;
				else if(sym == 2)
					fftIn[r*(dataLength/2 + 1)+c] = -0.1 + I * 0.1;
				else
					fftIn[r*(dataLength/2 + 1)+c] = -0.1 - I * 0.1;
				//printf("[%lf, %lf] ", creal(fftIn[r*(dataLength/2 + 1)+c]), cimag(fftIn[r*(dataLength/2 + 1)+c]));
			}
			for(int g = dataLength/2 + 1 - guard;g < dataLength/2 + 1;g++) {
				fftIn[r*(dataLength/2 + 1)+g] = 0 + I * 0;
				//printf("[%lf, %lf] ", creal(fftIn[r*(dataLength/2 + 1)+g]), cimag(fftIn[r*(dataLength/2 + 1)+g]));
			}
			//printf("\n");
		}
		
		// serial to parallel + IFFT (Hermitian symmetry applied inside the function) + parallel to serial
		// sizes must match the size of real matrix
		int size1 = numCarriers;
		int size2 = dataLength;
		fftw_plan txPlan = fftw_plan_dft_c2r_2d(size1, size2, fftIn, fftOut, FFTW_ESTIMATE);
		fftw_execute(txPlan);
		
		for(int c = 0;c < dataLength * numCarriers;c++) {
			//printf("%lf\n", fftOut[c]);
			if(fftOut[c] > high) high = fftOut[c];
			if(fftOut[c] < low) low = fftOut[c];
			fprintf(fileOut, "%lf\n", fftOut[c]);
		}

		fftw_destroy_plan (txPlan);
		free(fftIn);
		free(fftOut);
		free(symstream);
		
		frames++;
	}
	
	//average value is always 0	
	printf("Total Frames : %d\n", frames);
	printf("Highest real value : %lf\n", high);
	printf("Lowest real value : %lf\n", low);
	printf("Length of real value : %d\n", frames * numCarriers * dataLength);
	
	fclose(fileOut);
	fclose(fileIn);
	return 0;
}
