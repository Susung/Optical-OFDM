# Optical-OFDM

Install FFTW from
http://www.fftw.org/

compile with
  gcc -Wall -o ofdmTx ofdmTx.c -lfftw3 -lm & gcc -Wall -o ofdmRx ofdmRx.c -lfftw3 -lm

run by
	./ofdmTx
	./ofdmRx
