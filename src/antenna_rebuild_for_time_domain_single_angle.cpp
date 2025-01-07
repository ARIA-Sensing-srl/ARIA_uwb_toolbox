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
## @deftypefn {} {@var{aout} =} antenna_rebuild_for_time_domain_single_angle (@var{antenna_input},@var{tmax},@var{ts}, @var{az}, @var{zen})
##Resample the antenna to be compliant with a time-domain signal, along a preferred direction
##@var{antenna_input} is the input antenna \n\
##@var{tmax} is a single value containing the maximum time for the time-domain signal
##@var{ts} is the time sampling of the time-domain signal
##@var{azimuth} is the azimuth angle of interest
##@var{zenith} is the zenith angle of interest
##@end deftypefn

## Author: Alessio Cacciatori <alessioc@alessio-laptop>
## Created: 2024-11-15
*/

#include <octave/oct.h>
#include <octave/ov-struct.h>
#include <octave/parse.h>
#include "aria_uwb_toolbox.h"

octave_value ant_build_time_domain_angle(const octave_value_list& args)
{
    // This procedure do not check for data!!!
    octave_map  ant = args(0).map_value();
    NDArray     tmax_array = args(1).array_value();
    double      tmax = tmax_array(0);
    NDArray     ts_array = args(2).array_value();

    // If we don't have directivity data, let's calculate
    if (!(ant.contains("aeff_p"))||(!(ant.contains("aeff_t"))))
        ant = directivity(octave_value_list(ant)).map_value();

    double ts = ts_array(0);
    int nsamples = std::floor(tmax / ts);

    double df = 1.0/(double(nsamples)*ts);
    NDArray freq_of_interests;
    // Data for 0 pre-fill
    ComplexNDArray ep_ant =ant.getfield("ep")(0).complex_array_value();
    ComplexNDArray et_ant =ant.getfield("et")(0).complex_array_value();
    ComplexNDArray aeffp_ant =ant.getfield("aeff_p")(0).complex_array_value();
    ComplexNDArray aefft_ant =ant.getfield("aeff_t")(0).complex_array_value();
    NDArray        dir_ant   =ant.getfield("dir_abs")(0).array_value();

    NDArray        freq_ant = ant.getfield("freq")(0).array_value();
    octave_value   azimuth = ant.getfield("azimuth")(0);
    octave_value   zenith  = ant.getfield("zenith")(0);
    octave_value_list interp_args;
    interp_args.resize(9);
    interp_args(0) = azimuth;
    interp_args(1) = zenith;
    interp_args(2) = freq_ant;

    interp_args(4) = args(3);
    interp_args(5) = args(4);
    interp_args(6) = freq_ant;
    interp_args(7) = "linear";
    interp_args(8) = 0;

    // First of all, interpolate fields down to desired angles

    interp_args(3) = ep_ant;
    ep_ant =  octave::feval("interpn",interp_args)(0).complex_array_value();

    interp_args(3) = et_ant;
    et_ant =  octave::feval("interpn",interp_args)(0).complex_array_value();

    interp_args(3) = aeffp_ant;
    aeffp_ant =  octave::feval("interpn",interp_args)(0).complex_array_value();

    interp_args(3) = aefft_ant;
    aefft_ant =  octave::feval("interpn",interp_args)(0).complex_array_value();

    interp_args(3) = dir_ant;
    dir_ant =  octave::feval("interpn",interp_args)(0).array_value();

    int         nf_start  = freq_ant.numel();
    double      f_ant_min = freq_ant.min()(0);
    double      f_ant_max = freq_ant.max()(0);
    double      f_skip = 500e6;
    double      fsim_max = 1.0/(ts*2.0);
    int         extra_freqs_low;
    int         extra_freqs_high;
    if (f_ant_min < df)
        extra_freqs_low  = 0;
    else
        extra_freqs_low = 2;

    if (f_ant_max > fsim_max)
        extra_freqs_high = 0;
    else
        extra_freqs_high = 2;



    // Add two points
    NDArray freq_start;
    ComplexNDArray ep_start;
    ComplexNDArray et_start;
    ComplexNDArray aeffp_start;
    ComplexNDArray aefft_start;
    NDArray dir_start;

    ep_start.resize(dim_vector({1, nf_start + extra_freqs_low + extra_freqs_high}));
    et_start.resize(dim_vector({1, nf_start + extra_freqs_low + extra_freqs_high}));
    aeffp_start.resize(dim_vector({1, nf_start + extra_freqs_low+ extra_freqs_high}));
    aefft_start.resize(dim_vector({1, nf_start + extra_freqs_low + extra_freqs_high}));
    freq_start.resize(dim_vector({1,nf_start+extra_freqs_low+ extra_freqs_high}));
    dir_start.resize(dim_vector({1,nf_start+extra_freqs_low+ extra_freqs_high}));

    // from 0 to freq_min/2, put zeros in field
    // Pre

    if (extra_freqs_low==2)
    {
        double fminskip =f_ant_min - f_skip;
        double fminhalf =f_ant_min / 2.0;
        freq_start(0) = 0;
        freq_start(1) = fminskip > fminhalf ? fminskip : fminhalf;
        ep_start(1) = ep_start(0) = std::complex<double>(0.0,0.0);
        et_start(1) = et_start(0) = std::complex<double>(0.0,0.0);
        aeffp_start(1) = aeffp_start(0) = std::complex<double>(0.0,0.0);
        aefft_start(1) = aefft_start(0) = std::complex<double>(0.0,0.0);
        dir_start(1) = dir_start(0) = 0.0;
    }

    if (extra_freqs_high == 2)
    {
        int n_end = nf_start+extra_freqs_low+ extra_freqs_high-1;
        int n_end_1 = n_end - 1;

        double fmaxskip =f_ant_max + f_skip;
        double fmaxhalf =(f_ant_max + fsim_max) / 2.0;

        freq_start(n_end)   = 0;
        freq_start(n_end_1) = fmaxskip < fmaxhalf ? fmaxskip : fmaxhalf;
        ep_start(n_end) = ep_start(n_end_1) = std::complex<double>(0.0,0.0);
        et_start(n_end) = et_start(n_end_1) = std::complex<double>(0.0,0.0);
        aeffp_start(n_end) = aeffp_start(n_end_1) = std::complex<double>(0.0,0.0);
        aefft_start(n_end) = aefft_start(n_end_1) = std::complex<double>(0.0,0.0);
        dir_start(1) = dir_start(0) = 0.0;
    }

    int fft_half_max;
    if (nsamples % 2==1)
    {
        // Let's build only the "meaningful" part of the FFT
        fft_half_max = (nsamples - 1)/2;
        freq_of_interests.resize(dim_vector({1, fft_half_max+1}));
        for (int f=0; f <= fft_half_max; f++)
            freq_of_interests(f)= (double)f * df;

        // NB The upper half index is mapped to lower : upper(i)->n-upper(i), with
        // upper(0) = (nsamples-1)/2
        // upper(1) = (nsamples - 1)/2 - 1
        // ....
        // upper(n-1) = 1
    }
    else
    {
        // Let's build only the "meaningful" part of the FFT
        fft_half_max = (nsamples)/2;
        freq_of_interests.resize(dim_vector({1, fft_half_max+1}));
        for (int f=0; f <= fft_half_max; f++)
            freq_of_interests(f)= (double)f * df;

        // NB The upper half index is mapped to lower : upper(i)->n-upper(i), with
        // upper(0) = (nsamples/2) -1
        // upper(1) = (nsamples/2) -2
        // ....
        // upper(n-1) = 1
    }


    // Fill with prev data and compensate for reference distance
    double delay = REF_DISTANCE/C0;

    for (int fs=0,f = extra_freqs_low; fs < nf_start; fs++, f++)
    {
        double curr_freq= freq_ant(fs);
        freq_start(f)   = curr_freq;
        std::complex<double> comp_delay = std::exp(std::complex(0.0, M_2PI * curr_freq * delay));
        ep_start(f)     = ep_ant(fs) * comp_delay;
        et_start(f)     = et_ant(fs) * comp_delay;
        aeffp_start(f)  = aeffp_ant(fs) * comp_delay;
        aefft_start(f)  = aefft_ant(fs) * comp_delay;
        dir_start(f)    = dir_ant(fs);
    }

    // Now we just need to interpolate over desired freq range
    ComplexNDArray epi  = octave::feval("interp1",octave_value_list({freq_start, ep_start, freq_of_interests, "linear", 0}))(0).complex_array_value();
    ComplexNDArray eti  = octave::feval("interp1",octave_value_list({freq_start, et_start, freq_of_interests, "linear", 0}))(0).complex_array_value();
    ComplexNDArray aepi = octave::feval("interp1",octave_value_list({freq_start, aeffp_start, freq_of_interests, "linear", 0}))(0).complex_array_value();
    ComplexNDArray aeti = octave::feval("interp1",octave_value_list({freq_start, aefft_start, freq_of_interests, "linear", 0}))(0).complex_array_value();
    NDArray        diri = octave::feval("interp1",octave_value_list({freq_start, dir_start, freq_of_interests, "linear", 0}))(0).array_value();
    int n_foi = freq_of_interests.numel();
    std::complex<double> comp_delay = std::complex<double>(1.0,0.0);
    std::complex<double> dexp       = std::exp(std::complex(0.0, -M_2PI * df * delay));
    for (int fi = 0; fi < n_foi; fi++)
    {
        epi(fi)*=comp_delay;
        eti(fi)*=comp_delay;
        aepi(fi)*=comp_delay;
        aeti(fi)*=comp_delay;
        comp_delay *= dexp;
    }

    ant.assign("td_freqs",  octave_value(freq_of_interests));
    ant.assign("td_ep",     octave_value(epi));
    ant.assign("td_et",     octave_value(eti));
    ant.assign("td_aeffp",  octave_value(aepi));
    ant.assign("td_aefft",  octave_value(aeti));

    ant.assign("td_az",      azimuth);
    ant.assign("td_zen",     zenith);

    ant.assign("td_tmax",   octave_value(tmax_array));
    ant.assign("td_ts",     octave_value(ts_array));
    ant.assign("n_ffts",    octave_value(nsamples));

    ant.assign("td_dir_abs",octave_value(diri));
    return octave_value(ant);
}

DEFUN_DLD(antenna_rebuild_for_time_domain_single_angle, args, , "-*- texinfo -*-\n\
@deftypefn {} {@var{aout} =} antenna_rebuild_for_time_domain (@var{antenna_input},@var{tmax},@var{ts})\n\
Resample the antenna to be compliant with a time-domain signal\n\
@var{antenna_input} is the input antenna \n\
@var{tmax} is a single value containing the maximum time for the time-domain signal \n\
@var{ts} is the time sampling of the time-domain signal \n\
@var{azimuth} is the azimuth angle of interest \n\
@var{zenith} is the zenith angle of interest \n\
@end deftypefn")
{
    if (args.length() != 5)
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

    if (!args(1).isreal())
    {
        error("tmax must be a real value");
        return octave_value();
    }

    NDArray tmax_array = args(1).array_value();
    if ((tmax_array.numel()!=1)||(tmax_array(0)<=0.0))
    {
        error("tmax must be a single positive value");
        return octave_value();
    }

    if (!args(2).isreal())
    {
        error("ts must be a real value");
        return octave_value();
    }

    NDArray ts_array = args(2).array_value();
    if ((ts_array.numel()!=1)||(ts_array(0)<=0.0))
    {
        error("ts must be a single positive value");
        return octave_value();
    }

    if ((!args(3).isreal())||(args(3).numel()!=1))
    {
        error("azimuth must be a single real value");
        return octave_value();
    }

    if ((!args(4).isreal())||(args(4).numel()!=1))
    {
        error("zenith must be a single real value");
        return octave_value();
    }
    // End of input check
    //----------------------------------------

    return ant_build_time_domain_angle(args);
}
