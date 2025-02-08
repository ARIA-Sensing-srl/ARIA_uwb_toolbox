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
## @deftypefn {} {@var{retval} =} antenna_radiated_power (@var{antenna_input},@var{freq_time},@var{signal})
## @var{antenna_input} is the input antenna
## @var{freq_time} is either the time domain support for @var{signal} input or the frequency support. If empty
## the computation is performed at the antenna frequencies
## @var{signal} is the time domain signal
##
## @seealso{}
## @end deftypefn

## Author: Alessio Cacciatori <alessioc@alessio-laptop>
## Created: 2024-11-15
*/

#include <octave/oct.h>
#include <octave/ov-struct.h>
#include <octave/parse.h>
#include "aria_uwb_toolbox.h"

DEFUN_DLD(antenna_radiated_power, args, , "-*- texinfo -*-\n\
@deftypefn {} {@var{aout} =} antenna_radiated_power (@var{antenna_input},@var{freq},@var{signal})\n\
Calculate the radiated power.\n\
@var{antenna_input} is the input antenna \n\
@var{freq} is either the frequency sampling for @var{signal} input or the frequency support. If empty \n\
the computation is performed at the antenna frequencies \n\
@var{signal} is the time domain signal. If provided, @var{freq} must be one single value \n\
@end deftypefn")
{

    if (args.length() < 1)
    {
        print_usage();
        return octave_value();
    }
    octave_value ant_dut = args(0);

    octave_value check = octave::feval("antenna_is_valid",ant_dut)(0);
    if (!check.isempty())
    {
        octave_stdout << check.char_array_value();
        return octave_value();
    }

    octave_map ant = ant_dut.map_value();
    ComplexNDArray ep = ant.getfield("ep")(0).complex_array_value();
    ComplexNDArray et = ant.getfield("et")(0).complex_array_value();
    NDArray az = ant.getfield("azimuth")(0).array_value();
    int naz = az.numel();
    NDArray zen= ant.getfield("zenith")(0).array_value();
    int nzen= zen.numel();
    double dz = zen(1)-zen(0);
    double da = az(1)-az(0);
    NDArray freq =ant.getfield("freq")(0).array_value();
    // if we don't have a support, use native frequencies
    if (args.length()==1)
    {
        NDArray opwr;
        int nf = freq.numel();

        opwr.resize(dim_vector({1,nf}));

        for (int f=0; f < nf; f++)
        {
            double pwr = 0.0;
            for (int z=0; z < nzen; z++ )
            {
                double k_da_dz = da*dz*fabs(sin(zen(z)+dz/2));
                for (int a=0; a < naz; a++)
                {
					pwr += k_da_dz* (std::norm(ep.xelem(a,z,f)) + std::norm(et.xelem(a,z,f)))*ONE_OVER_ETA0;
                }
            }
            opwr(f)=pwr;
        }
        return octave_value(opwr);
    }
    // if we have spectrum, we interpolate
    if ((args.length()==2)||(args(2).array_value().numel()==0))
    {
        octave_value target_freq = args(1);
        ComplexNDArray epi = interp_field(ep, freq, target_freq).complex_array_value();
        ComplexNDArray eti = interp_field(et, freq, target_freq).complex_array_value();

        NDArray opwr;
        int nf = target_freq.numel();

        opwr.resize(dim_vector({1,nf}));

        for (int f=0; f < nf; f++)
        {
            double pwr = 0.0;

            for (int z=0; z < nzen; z++ )
            {
				double k_da_dz = da*dz*fabs(sin(zen.xelem(z)+dz/2));
                for (int a=0; a < naz; a++)
                {
					pwr += k_da_dz* (std::norm(epi.xelem(a,z,f)) + std::norm(eti.xelem(a,z,f)))*ONE_OVER_ETA0;
                }
            }

            opwr(f)=pwr;
        }
        return octave_value(opwr);

    }
    // if we have time, we need to calculate FFT at desired frequencies
    // Frequency, in the FFT domain is i * FS/N
    NDArray fs_array = args(1).array_value();
    if (fs_array.numel()!=1)
    {
        error("Please provide sampling frequency");
        return octave_value();
    }

    double  fs   = fs_array(0);
    double  fmax = freq.max()(0);
    double  fmin = freq.min()(0);

    // Build the frequency array that is inside fmin and fmax and whose freqs corresponds to FFT

    ComplexNDArray fft_signal = args(2).array_value().fourier();
    int n_ffts = fft_signal.numel();
    int n_useful_fft_samples = 0;
    double df = fs / n_ffts;
    for (int f=0; f < n_ffts; f++)
    {
        double current_freq = (double)(f)*df;
        if ((current_freq >= fmin)&&(current_freq <= fmax ))
        {
            n_useful_fft_samples++;
        }
        if (current_freq > fmax)
            break;
    }

    NDArray freq_of_interest(dim_vector({1,n_useful_fft_samples}));
    ComplexNDArray fft_of_interest(dim_vector({1,n_useful_fft_samples}));

    int fi=0;
    for (int f=0; f < n_ffts; f++)
    {
        double current_freq = (double)(f)*df;

        if ((current_freq >= fmin)&&(current_freq <= fmax ))
        {
			freq_of_interest.xelem(fi) = current_freq;
			fft_of_interest.xelem(fi++)= fft_signal(f);
        }
        if (current_freq > fmax)
            break;
    }

    ComplexNDArray epi = interp_field(ep, freq, freq_of_interest).complex_array_value();
    ComplexNDArray eti = interp_field(et, freq, freq_of_interest).complex_array_value();
    NDArray opwr;

    opwr.resize(dim_vector({1,n_useful_fft_samples}));

    for (int f=0; f < n_useful_fft_samples; f++)
    {
        double pwr = 0.0;
        for (int z=0; z < nzen; z++ )
        {
            double k_da_dz = da*dz*fabs(sin(zen(z)+dz/2));
            for (int a=0; a < naz; a++)
            {
				pwr += std::norm(fft_of_interest.xelem(f))*k_da_dz*(std::norm(epi.xelem(a,z,f)) + std::norm(eti.xelem(a,z,f)))*ONE_OVER_ETA0;
            }
        }
        opwr(f)=pwr;
    }

    return octave_value(opwr);
}
