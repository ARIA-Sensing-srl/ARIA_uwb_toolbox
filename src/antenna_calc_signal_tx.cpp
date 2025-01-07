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
##@deftypefn {} {@var{aout} =} antenna_calc_signal_tx (@var{ant},@var{pos_ant}, @var{pos}, @var{signal}, @var{ts})
##Calculate the signal transmitted (ep, et) from the antenna to a certain point
##@var{ant} is the antenna,
##@var{pos} is the position of the antenna
##@var{signal} is the signal. Please note that input ref. impedance is 50Ohm
##@var{ts}     is the sampling interval for the input signal
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

DEFUN_DLD(antenna_calc_signal_tx, args, , "-*- texinfo -*-\n\
@deftypefn {} {@var{aout} =} antenna_calc_signal_tx (@var{ant},@var{pos_ant}, @var{pos}, @var{signal}, @var{ts})\n\
Calculate the signal transmitted (ep, et) from the antenna to a certain point\n\
@var{ant} is the antenna, \n\
@var{pos_ant} is the position of the antenna \n\
@var{pos}     is the position where we calculated the radiated field \n\
@var{signal}  is the signal. Please note that input ref. impedance is 50Ohm \n\
@var{ts}      is the sampling interval for the input signal \n\
@end deftypefn")
{
    if (args.length() != 5)
    {
        print_usage();
        return octave_value_list();
    }
    octave_value ant_dut = args(0);

    octave_value check = octave::feval("antenna_is_valid",ant_dut)(0);
    if (!check.isempty())
    {
        octave_stdout << check.char_array_value();
        return octave_value();
    }
    octave_map ant = ant_dut.map_value();

    if (!(args(1).isreal())||(args(1).numel()!=3))
    {
        error("pos_ant must be a 3-values real vector");
    }

    if (!(args(2).isreal())||(args(2).numel()!=3))
    {
        error("pos must be a 3-values real vector");
    }

    if (args(3).numel()<2)
    {
        error("signal must contain at least two samples");
    }

    if (!(args(4).isreal())||(args(4).numel()!=1)||(args(4).array_value()(0)<=0))
    {
        error("ts must be a single positive real value");
    }


    NDArray pos_ant = args(1).array_value();
    NDArray pos     = args(2).array_value();

    // Make same dims
    if (pos.dim1()!=pos_ant.dim1())
        pos.reshape(pos_ant.dims());

    double  ts      = args(4).array_value()(0);

    // Delay
    NDArray delta   = (pos - pos_ant);
    double  az, zen, r;

    rect_to_polar(delta(0),delta(1),delta(2), az, zen, r);

    int     n_samples= args(3).numel();

    // Check if we need to update
    bool recalc =
       ((!ant.contains("td_freqs"))  ||
        (!ant.contains("td_ep"))     ||
        (!ant.contains("td_et"))     ||
        (!ant.contains("td_aeffp"))  ||
        (!ant.contains("td_aefft"))  ||
        (!ant.contains("td_az"))     ||
        (!ant.contains("td_zen"))    ||
        (!ant.contains("td_tmax"))   ||
        (!ant.contains("td_ts"))     ||
        (!ant.contains("n_ffts")));


    if (!recalc)
    {

        octave_value nfft = ant.getfield("n_ffts")(0);
        if (nfft.numel()!=1)
            recalc = true;
        else
        {
            if (nfft.array_value()(0)!=n_samples)
                recalc = true;
        }
    }

    if (!recalc)
    {

        octave_value azfield = ant.getfield("td_az")(0);
        if (azfield.numel()!=1)
            recalc = true;
        else
        {
            if (fabs((azfield.array_value()(0)-az))>1e-3)
                recalc = true;
        }
    }

    if (!recalc)
    {

        octave_value zenfield = ant.getfield("td_zen")(0);
        if (zenfield.numel()!=1)
            recalc = true;
        else
        {
            if (fabs((zenfield.array_value()(0)-zen))>1e-3)
                recalc = true;
        }
    }

    if (!recalc)
    {
        octave_value tsfield = ant.getfield("td_ts")(0);
        if (tsfield.numel()!=1)
            recalc = true;
        else
        {
            if (fabs((tsfield.array_value()(0)-ts))>1e-15)
                recalc = true;
        }
    }

    double az_min = ant.getfield("azimuth")(0).array_value().min()(0);
    double az_max = ant.getfield("azimuth")(0).array_value().max()(0);

    double zen_min = ant.getfield("zenith")(0).array_value().min()(0);
    double zen_max = ant.getfield("zenith")(0).array_value().max()(0);


    if (az < az_min)
        az+=M_2PI;

    if (az > az_max)
        az-=M_2PI;

    if (zen < zen_min)
        zen+=M_2PI;

    if (zen > zen_max)
        zen-=M_2PI;

    if (recalc)
    {
        octave_value_list params;
        params.resize(5);
        params(0) = ant_dut;
        params(1) = NDArray(dim_vector({1,1}), ts * double(n_samples));
        params(2) = NDArray(dim_vector({1,1}), ts);
        params(3) = NDArray(dim_vector({1,1}), az);
        params(4) = NDArray(dim_vector({1,1}), zen);
        ant = ant_build_time_domain_angle(params).map_value();
    }

    // Antenna field is supposed calculated from a source that deliver available power = 1W
    // to a matched antenna (50Ohm). The implicit voltage at antenna terminals is therefore
    // a set of Vpk= sqrt(1W * 2 * 50) sine input.
    // The input signal is considered as voltage over a 50 Ohm load, so we have to scale by the
    // implicit voltage that we assumed during antenna creation / loading
    ComplexNDArray in_fft = args(3).complex_array_value().fourier()/(r*sqrt(100));

    ComplexNDArray out_fft_ep, out_fft_et;
    ComplexNDArray epi = ant.getfield("td_ep")(0).complex_array_value();
    ComplexNDArray eti = ant.getfield("td_et")(0).complex_array_value();

    out_fft_ep.resize(in_fft.dims());
    out_fft_et.resize(in_fft.dims());

    NDArray freqs_fft = ant.getfield("td_freqs")(0).array_value();
    if (freqs_fft.numel() < 2)
    {
        error("Error: only one frequency available");
        return octave_value_list();
    }

    double df = freqs_fft(1)-freqs_fft(0);

    int n_freq_of_interest = freqs_fft.numel();

    std::complex<double> comp_delay = std::complex<double>(1.0,0.0);
    std::complex<double> dexp       = std::exp(std::complex(0.0, -M_2PI * df * (r-REF_DISTANCE)/C0));

    for (int f =0 ; f < n_freq_of_interest; f++)
    {
        out_fft_ep(f) = in_fft(f) * epi(f) * comp_delay;
        out_fft_et(f) = in_fft(f) * eti(f) * comp_delay;
        comp_delay *= dexp;
    }

    // Build remaining part of the signal
    int nstart = (n_samples % 2)==0 ? n_samples/2 -1 : (n_samples-1)/2;

    for (int f=n_freq_of_interest; f < n_samples; f++, nstart--)
    {
        out_fft_ep(f) = std::conj(out_fft_ep(nstart));
        out_fft_et(f) = std::conj(out_fft_et(nstart));
    }

    ant.assign("td_tx_ep",octave_value(out_fft_ep.ifourier()));
    ant.assign("td_tx_et",octave_value(out_fft_et.ifourier()));


    return octave_value_list(ant);
}
