/* Copyright (C) 2024 Alessio Cacciatori
##
## This program is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program.  If not, see <https://www.gnu.org/licenses/>.

## -*- texinfo -*-
## @deftypefn {} @var{aout} = pm_demod (@var{signal},@var{ts},@var{center_freq})\n\
## Phase Demodulator for PN calculations. Output Phase spectrum is onesided\n\
## @var{signal} is the signal for which we calculate the PN \n\
## @var{ts} is the sampling interval for @var{signal} \n\
## @var{center_freq} is the center frequency. \n\
## @var{aout}: is the demodulated phase
## @end deftypefn"
## Author: Alessio Cacciatori <alessioc@alessio-laptop>
## Created: 2024-11-15
*/
#include <octave/oct.h>
#include <octave/ov-struct.h>
#include <octave/parse.h>
#include "aria_uwb_toolbox.h"

DEFUN_DLD(pm_demod, args, , "-*- texinfo -*-\n\
@deftypefn {} @var{aout} = pm_demod (@var{signal},@var{ts},@var{center_freq})\n\
Phase Demodulator for PN calculations. Output PN pectrum is onesided\n\
@var{signal} is the signal for which we calculate the PN \n\
@var{ts} is the sampling interval for @var{signal} \n\
@var(center_freq) is the center frequency.\n\
@end deftypefn")
{
    if (args.length()!=3)
    {
        print_usage();
        return octave_value();
    }

    if (args(0).numel()<1)
    {
        error("signal must contain at least two samples");
        return octave_value();
    }
    ComplexNDArray signal = args(0).complex_array_value();
    dim_vector dims = signal.dims();
    if (dims.num_ones()>1)
    {
        error("signal must be a vector");
        return octave_value();
    }

    if ((!args(1).isreal())||(args(1).numel()!=1)||
        (args(1).array_value()(0)<=0))
    {
        error("ts must be a single positive value");
        return octave_value();
    }

    if ((!args(2).isreal())||(args(2).numel()!=1)||
        (args(2).array_value()(0)<=0))
    {
        error("center_freq must be a single positive value");
        return octave_value();
    }


    int nsamples = signal.numel();
    double ts = args(1).array_value()(0);
    double center_freq = args(2).array_value()(0);

    std::complex<double> factor(1.0,0.0);
    std::complex<double> dfactor = std::exp(std::complex<double>(0.0, -M_2PI*center_freq*ts));

    ComplexNDArray demod;
    demod.resize(signal.dims());

    for (int n=0; n < nsamples; n++, factor*=dfactor)
        demod(n) = factor*signal(n);

    // Set the cutoff filter to Fcenter
    octave_value_list filter_in;
    filter_in.append(octave_value(8));
    filter_in.append(octave_value((center_freq/2.0)/(1.0/(2.0*ts))));
    octave_value_list filter_out = octave::feval("butter",filter_in,2);
    filter_out.append(octave_value(demod));

    ComplexNDArray out = octave::feval("filtfilt",filter_out)(0).complex_array_value();

    ComplexNDArray phase_out(out.dims());

    double phase_prev = 0.0;
    double phase = 0.0;

    for (int n=0; n < nsamples; n++, phase_prev = phase)
    {
        std::complex<double> curr= out(n);
        phase = atan2(curr.imag(), curr.real());

        double d = phase-phase_prev;
        if (d > M_PI)
        {
            phase = phase - M_2PI * std::ceil(d / M_2PI);
        }
        if (d < -M_PI)
        {
            phase = phase + M_2PI * std::ceil  (-d/ M_2PI);

        }
        phase_out(n)=phase;
    }
    return octave_value(phase_out);

}
