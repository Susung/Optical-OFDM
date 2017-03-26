#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <complex.h>
#include <fftw3.h>
#include <math.h>
#include <time.h>

int main()

/******************************************************************************/
/*
  Purpose:

    TEST04: apply FFT to real 2D data.

  Discussion:

    In this example, we generate NX=8 by NY=10 random real values 
    stored as an NX by NY array of type DOUBLE named "IN".

    We have FFTW3 compute the Fourier transform of this data named "OUT".

    We have FFTW3 compute the inverse Fourier transform of "OUT" to get
    "IN2", which should be the original input data, scaled by NX * NY.

    The Fourier coefficients are stored in an NX by NYH array where
    NYH = (NY/2) + 1.  We only compute about half the data because
    of real data implies symmetric FFT coefficients.

      a[i*nyh+j][0] is the real      part of A(I,J).
      a[i*nyh+j][1] is the imaginary part of A(I,J)..

  Licensing:

    This code is distributed under the GNU LGPL license. 

  Modified:

    05 November 2007

  Author:

    John Burkardt
*/
{
  int i;
  double *in;
  double *in2;
  int j;
  int nx = 8;
  int ny = 10;
  int nyh;
  fftw_complex *out;
  fftw_plan plan_backward;
  fftw_plan plan_forward;
  unsigned int seed = 123456789;

  printf ( "\n" );
  printf ( "TEST04\n" );
  printf ( "  Demonstrate FFTW3 on a %d by %d array of real data.\n",
    nx, ny );
  printf ( "\n" );
  printf ( "  Transform data to FFT coefficients.\n" );
  printf ( "  Backtransform FFT coefficients to recover data.\n" );
  printf ( "  Compare recovered data to original data.\n" );
/*
  Create the input array, an NX by NY array of doubles.
*/
  in = ( double * ) malloc ( sizeof ( double ) * nx * ny );

  srand ( seed );

  for ( i = 0; i < nx; i++ )
  {
    for ( j = 0; j < ny; j++ )
    {
      in[i*ny+j] = rand ( );
    }
  }

  printf ( "\n" );
  printf ( "  Input Data:\n" );
  printf ( "\n" );

  for ( i = 0; i < nx; i++ )
  {
    for ( j = 0; j < ny; j++ )
    {
      printf ( "  %4d  %4d  %12f\n", i, j, in[i*ny+j] );
    }
  }
/*
  Create the output array OUT, which is of type FFTW_COMPLEX,
  and of a size NX * NYH that is roughly half the dimension of the input data
  (ignoring the fact that the input data is real, and the FFT
  coefficients are complex).
*/
  nyh = ( ny / 2 ) + 1;

  out = fftw_malloc ( sizeof ( fftw_complex ) * nx * nyh );

  plan_forward = fftw_plan_dft_r2c_2d ( nx, ny, in, out, FFTW_ESTIMATE );

  fftw_execute ( plan_forward );

  printf ( "\n" );
  printf ( "  Output FFT Coefficients:\n" );
  printf ( "\n" );

  for ( i = 0; i < nx; i++ )
  {
    for ( j = 0; j < nyh; j++ )
    {
      printf ( "  %4d  %4d  %12f  %12f\n", 
      i, j, creal(out[i*nyh+j]), cimag(out[i*nyh+j]));
    }
  }
/*
  Recreate the input array.
*/
  in2 = ( double * ) malloc ( sizeof ( double ) * nx * ny );

  plan_backward = fftw_plan_dft_c2r_2d ( nx, ny, out, in2, FFTW_ESTIMATE );

  fftw_execute ( plan_backward );

  printf ( "\n" );
  printf ( "  Recovered input data divided by NX * NY:\n" );
  printf ( "\n" );

  for ( i = 0; i < nx; i++ )
  {
    for ( j = 0; j < ny; j++ )
    {
      printf ( "  %4d  %4d  %12f\n",  
        i, j, in2[i*ny+j] / ( double ) ( nx * ny ) );
    }
  }
/*
  Free up the allocated memory.
*/
  fftw_destroy_plan ( plan_forward );
  fftw_destroy_plan ( plan_backward );

  free ( in );
  free ( in2 );
  fftw_free ( out );

  return 0;
}
