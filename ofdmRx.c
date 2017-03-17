		//serial to parralel + FFT + parallel to serial
		fftw_complex* rxOut =fftw_malloc(sizeof(fftw_complex) * numCarriers * (FRAME_DATA_SIZE_BYTES/2 + 1));
		fftw_plan rxPlan = fftw_plan_dft_r2c_2d(size1, size2, fftOut, rxOut, FFTW_ESTIMATE);
		fftw_execute(rxPlan);

		for(int c = 0;c < numCarriers * (FRAME_DATA_SIZE_BYTES/2 + 1);c++) {
			printf("%f, %f\n", creal(rxOut[c])/(numCarriers * FRAME_DATA_SIZE_BYTES), cimag(rxOut[c])/(numCarriers * FRAME_DATA_SIZE_BYTES));//need to normalize
		}
