# Optical-OFDM

Install FFTW from by following the instructions
http://www.fftw.org/

compile with
  `gcc -Wall -o ofdmTx ofdmTx.c -lfftw3 -lm & gcc -Wall -o ofdmRx ofdmRx.c -lfftw3 -lm`

run by
```
	./ofdmTx
	./ofdmRx
```

# How it works
Since we want to get real valued output from IFFT, the input complex matrix must have hermitian symmetry.

Fortunately, the IFFT from FFTW3 already takes care of the symmetry so there is no need to apply the symmetry manually.

Therefore, the total size of complex input must be : (# of subcarriers) * (FFT Length/2 + 1)

and the real valued output from the IFFT will be : (# of subcarriers) * (FFT Length)

FFTW3 also takes care of the parallel -> series conversion and vice versa, so as long as the dimensions are passed into the function, a 1-dimension array can be passed into the IFFT function.

The dimensions are row , column

Note about cyclic prefix :
For some reason, FFTW3 function converting real matrix to complex matrix alters first and last value in every subcarrier.

Usually in OFDM, guard bands are only inserted at the front of each subcarrier, but for this case, half of guard band length is inserted at both start and end of each subcarrier.

Guard band length is 25% of FFT length.
