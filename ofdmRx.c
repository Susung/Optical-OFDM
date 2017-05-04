#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <complex.h>
#include <fftw3.h>
#include <math.h>
#include <string.h>

int main(int argc, char*argv[]) {	
	FILE *fileIn;
	FILE *fileOut;
	char *input = "transmitted";
	char *output = "image_out.bmp";
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
	
	char *line = NULL;
	double *fftIn = (double*) malloc(sizeof(double) * numCarriers * dataLength);
	int i = 0;
	int guard = dataLength/16;
	unsigned char ptr;
	int k = 0;
	union {
		float a;
		unsigned char bytes[4];
	} converter;
	while(fread(&ptr, 1, 1, fileIn) != 0) {
		if(k < 4) {
			converter.bytes[k] = ptr;
			k++;
			if(k == 4) {
				k = 0;
			} else {
				continue;
			}
		}
		fftIn[i] = converter.a;
		//printf("%lf\n", fftIn[i]);
		i++;
		if(i == numCarriers * dataLength) {
			int size1 = numCarriers;
			int size2 = dataLength;
			//serial to parralel + FFT + parallel to serial
			fftw_complex* fftOut = fftw_malloc(sizeof(fftw_complex) * numCarriers * (dataLength/2 + 1));
			fftw_plan rxPlan = fftw_plan_dft_r2c_2d(size1, size2, fftIn, fftOut, FFTW_ESTIMATE);
			fftw_execute(rxPlan);
			
			unsigned int *symstream = (unsigned int*) malloc(sizeof(unsigned int) * numCarriers * (dataLength/2 + 1 - 2 * guard)); 

			for(int r = 0;r < numCarriers ;r++) {
				for(int c = guard;c < dataLength/2 + 1 - guard;c++) {
					fftw_complex comp = fftOut[r*(dataLength/2 + 1)+c];
					unsigned int sym;
					if(creal(comp) > 0 && cimag(comp) > 0) 
						sym = 0;
					else if(creal(comp) > 0 && cimag(comp) < 0)
						sym = 1;
					else if(creal(comp) < 0 && cimag(comp) > 0)
						sym = 2;
					else
						sym = 3;
					//printf("[%lf, %lf]\n", creal(comp), cimag(comp));
					symstream[r*(dataLength/2 + 1 - 2*guard)+c-guard] = sym;
				}
			}
			
			for(int b = 0;b < numCarriers/4 * (dataLength/2 + 1 - 2 * guard);b++) {
				unsigned char oneByte = (symstream[b * 4] << 6) + (symstream[b * 4 + 1] << 4) + (symstream[b * 4 + 2] << 2) + symstream[b * 4 + 3];
				//if(oneByte != 0) fprintf(fileOut, "%c", oneByte);
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

