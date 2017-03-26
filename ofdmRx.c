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
	char *input = "transmitted.txt";
	char *output = "output.txt";
	int dopt;
	int numCarriers = 64;
	int dataLength = 256;
	
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
	
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	double *fftIn = (double*) malloc(sizeof(double) * numCarriers * dataLength);
	int i = 0;
	while((read = getline(&line, &len, fileIn)) != -1) {
		char *ptr;
		double ret;
		ret = strtod(line, &ptr);
		fftIn[i] = ret;
		i++;
		if(i == numCarriers * dataLength) {
			int size1 = numCarriers;
			int size2 = dataLength;
			//serial to parralel + FFT + parallel to serial
			fftw_complex* fftOut =fftw_malloc(sizeof(fftw_complex) * numCarriers * (dataLength/2 + 1));
			fftw_plan rxPlan = fftw_plan_dft_r2c_2d(size1, size2, fftIn, fftOut, FFTW_ESTIMATE);
			fftw_execute(rxPlan);
			
			unsigned int *symstream = (unsigned int*) malloc(sizeof(unsigned int) * numCarriers * (dataLength/2 + 1)); 

			for(int c = 0;c < numCarriers * (dataLength/2 + 1);c++) {
				unsigned int sym;
				if(creal(fftOut[c]) > 0.707 && cimag(fftOut[c]) > 0.707) 
					sym = 0;
				else if(creal(fftOut[c]) > 0.707 && cimag(fftOut[c]) < 0.707)
					sym = 1;
				else if(creal(fftOut[c]) < 0.707 && cimag(fftOut[c]) > 0.707)
					sym = 2;
				else if(creal(fftOut[c]) < 0.707 && cimag(fftOut[c]) < 0.707)
					sym = 3;
				else
					sym = 2;
				symstream[c] = sym;
			}
			
			for(int b = 0;b < numCarriers/4 * (dataLength/2 + 1);b++) {
				unsigned char oneByte = (symstream[b * 4] << 6) + (symstream[b * 4 + 1] << 4) + (symstream[b * 4 + 2] << 2) + symstream[b * 4 + 3];
				if(oneByte == 0) continue;
				fprintf(fileOut, "%c", oneByte);
			}
			
			free(fftOut);
			free(symstream);
			fftw_destroy_plan(rxPlan);
			i = 0;
		}
		
	}

	free(line);

	return 0;
}

