/* Copyright (c) 2010, 2011 CodeSourcery, Inc.  All rights reserved. */

/* Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

       * Redistributions of source code must retain the above copyright
         notice, this list of conditions and the following disclaimer.
       * Redistributions in binary form must reproduce the above
         copyright notice, this list of conditions and the following
         disclaimer in the documentation and/or other materials
         provided with the distribution.
       * Neither the name of CodeSourcery nor the names of its
         contributors may be used to endorse or promote products
         derived from this software without specific prior written
         permission.

   THIS SOFTWARE IS PROVIDED BY CODESOURCERY, INC. "AS IS" AND ANY
   EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL CODESOURCERY BE LIABLE FOR
   ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
   BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
   EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  */

/// Description
///   Determine the coefficients of a high-pass FIR filter and
///   use it to eliminate an unwanted signal component.

#include <iostream>
#include <vsip/initfin.hpp>
#include <vsip/support.hpp>
#include <vsip/signal.hpp>
#include <vsip/math.hpp>

#include "freqz.hpp"
#include "fir_window.hpp"
#include "modulate.hpp"


int
main(int argc, char **argv)
{
  using namespace vsip;
  
  vsipl init(argc, argv);

  // Create a high-pass filter with a cutoff frequency of 2/3rds
  // the Nyquist frequency of fs/2 (= pi).  First create a low-pass
  // filter with the mirror-image of the response we want (cutoff
  // at 1/3rd Nyquist).
  length_type M = 31;
  scalar_f wc = M_PI / 3;
  Vector<scalar_f> h = fir_window(wc, blackman(M));

  // Now modulate that by PI in order to mirror the low-pass
  // response and create a high-pass filter.  This changes the
  // cutoff frequency to (pi - pi/3) = 2/3 pi.
  h = modulate(h, M_PI);


  // Compute the frequency response (see high_pass.png).  The
  // feedback coefficients are the impulse response 'h' and the
  // feedforward coefficients are set to 1 as this is not an IIR.
  length_type N = 512;
  Vector<cscalar_f> H = freqz(h, 1, N);
  // TODO: reimplement
  // save_view_as<float>("high_pass1.view", mag(H));


  // Use the filter to eliminate one of a pair of signals, one 
  // below the cutoff frequency and one above.
  Vector<scalar_f> n = ramp<scalar_f>(0, 1, N);
  scalar_f w1 = M_PI * 1/6;
  scalar_f w2 = M_PI * 5/6;
  Vector<scalar_f> x1 = sin(w1*n);
  Vector<scalar_f> x2 = sin(w2*n + M_PI/4);
  Vector<scalar_f> x = x1 + x2;

  // Examine the spectrum before filtering (see high_pass_before.png)
  Fft<const_Vector, scalar_f, cscalar_f>
    fft(Domain<1>(N), 1.0);

  // Each frequency bin increases by 1/fs, so (n * 1/fs) is the
  // frequency in Hz.
  Vector<cscalar_f> X = fft(x);
  // TODO: reimplement
  // save_view_as<scalar_f>("high_pass2.view", mag(X));


  // Filter the signal
  Fir<scalar_f> hpf(h, N);
  Vector<scalar_f> y(N);
  hpf(x, y);


  // Examine the spectrum after filtering (see high_pass_after.png)
  // Note the lower frequency component is severely attenuated.
  Vector<cscalar_f> Y = fft(y);
  // TODO: reimplement
  // save_view_as<float>("high_pass3.view", mag(Y));
}
