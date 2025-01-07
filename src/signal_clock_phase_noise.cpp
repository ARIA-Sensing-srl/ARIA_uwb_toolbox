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
##@deftypefn {} {@var{aout} =} signal_clock_phase_noise (@var{time},@var{freqs},@var{phase_noise_dbc})\n\
##Create a sin signal with given PN spectral mask\n\
##@var{time} is the time support \n\
##@var{freqs} is a vector containing the frequency points at which the PN is specified\n\
##@var{phase_noise_dbc} is the phase noise in dBc\n\
##@end deftypefn

## Author: Alessio Cacciatori <alessioc@alessio-laptop>
## Created: 2024-11-15
*/
#include <octave/oct.h>
#include <octave/ov-struct.h>
#include <octave/parse.h>
#include "aria_uwb_toolbox.h"

DEFUN_DLD(signal_clock_phase_noise, args, , "-*- texinfo -*-\n\
@deftypefn {} {@var{aout} =} signal_clock_phase_noise (@var{time},@var{center_freq},@var{freqs},@var{phase_noise_dbc})\n\
Create a sin signal with given PN spectral mask\n\
@var{time} is the time support \n\
@var(center_freq) is the center frequency (it can be an array for freq error)\n\
@var{freqs} is a vector containing the frequency points at which the PN is specified\n\
@var{phase_noise_dbc} is the phase noise in dBc\n\
@end deftypefn")
{
    if (args.length() != 4)
    {
        print_usage();
        return octave_value();
    }

    if ((!args(0).isreal())||(args(0).numel()<2)||
        (!args(0).dims().isvector()))
    {
        error("time must be a vector with at least two samples");
        return octave_value();
    }
    if ((!args(1).isreal())||
        ((args(1).numel()!=1)&&(args(1).numel()!=args(0).numel())))
    {
        error("center_freq must be a single real value or a vector with a sample per each time");
        return octave_value();
    }

    NDArray center_freq = args(1).array_value();
    for (int f=0; f < center_freq.numel(); f++)
        if (center_freq(f) < 0)
        {
            error("center_freq must be positive");
            return octave_value();
        }

    if ((!args(2).isreal())||(args(2).numel()<2)||
        (!args(2).dims().isvector()))
    {
        error("freqs must be a vector with at least two points");
        return octave_value();
    }

    NDArray f_start = args(2).array_value();
    NDArray f_start_log(f_start.dims());
    for (int f=0; f < f_start.numel(); f++)
    {
        double f_act = f_start(f);
        if (f_act<=0)
        {
            error("Frequencies must be positive!");
            return octave_value();
        }
        f_start_log(f) = log10(f_act );
    }

    if ((!args(3).isreal())||(args(3).numel()<2)||
        (!args(3).dims().isvector()))
    {
        error("PN must be a vector with at least two points");
        return octave_value();
    }

    if (args(2).numel()!=args(3).numel())
    {
        error("PN and Freqs size must match");
        return octave_value();
    }

    NDArray time  = args(0).array_value();
    int n_samples = time.numel();
    NDArray dim_t(dim_vector({1,2}));
    dim_t(0)=1;
    dim_t(1)=n_samples;

    NDArray phase_error= (octave::feval("randn", octave_value_list(octave_value(dim_t)))(0).array_value())*sqrt(double(n_samples))/2.0;

    int i_start, i_max;

    if (n_samples%2==1)
    {
        i_max = (n_samples-1)/2;
        i_start=i_max;
    }
    else
    {
        i_max = n_samples/2;
        i_start=i_max-1;
    }

    double ts = time(2)-time(1);
    double df = 1/(ts*double(n_samples));

    NDArray freq_of_interest(dim_vector({1,i_max+1}));
    NDArray log_fois(freq_of_interest.dims());
    double  fmin = f_start.min()(0);
    for (int f=0; f <= i_max; f++)
    {
        double actualf = ((double)f)*df;
        freq_of_interest(f) = actualf;
        log_fois(f) = f==0 ? log10(fmin/1000) : log10(actualf);
    }

    // Perform a log interp
    NDArray pn_interp = octave::feval("interp1", octave_value_list({f_start_log, args(3),log_fois,"linear",0}))(0).array_value();
    double  pn_0 = args(3).array_value()(0);
    double  pn_1 = args(3).array_value()(args(3).numel()-1);

    // Extrapolate setting the closest limit
    double fmax = f_start.max()(0);
    for (int f=0; f <= i_max; f++)
    {
        double f_act = freq_of_interest(f);
        if (f_act < fmin)
            pn_interp(f) = pn_0;
        if (f_act > fmax)
            pn_interp(f) = pn_1;
    }
    // Multiply by PSD and get time
    ComplexNDArray phase_error_f = phase_error.fourier();
    for (int f=0; f <= i_max; f++)
        phase_error_f(f)*=pow(10.0,pn_interp(f)/20);
    // Build conj
    for (int f=i_max+1, fi = i_start; f < n_samples; f++, fi--)
        phase_error_f(f) = std::conj(phase_error_f(fi));

    ComplexNDArray phase_error_t = phase_error_f.ifourier();
    NDArray out_clock(time.dims());
    for (int t=0; t < n_samples; t++)
    {
        double cf = center_freq.numel()==1?center_freq(0) : center_freq(t);
        out_clock(t) = cos(M_2PI * cf * time(t) + std::real(phase_error_t(t)));
    }

    return octave_value(out_clock);
}
