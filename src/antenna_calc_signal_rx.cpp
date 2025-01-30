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
##@deftypefn {} {@var{aout} =} antenna_calc_signal_rx (@var{ant},@var{pos_ant}, @var{pos}, @var{signal}, @var{ts})\n\
## Calculate the signal received (ep, et) from the antenna to a certain point\n\
## @var{ant} is the antenna, \n\
## @var{pos}     is the position where we calculated the radiated field \n\
## @var{ts}      is the sampling interval for the input signal \n\
## @var{do_noise}   is 1 to add thermal noise \n\
## @var{rcs}        is the rcs matrix over frequency (2,2,n) \n\
## @var{rcs_freq}   is the rcs frequency support \n\
## @seealso{}
## @end deftypefn

## Author: Alessio Cacciatori <alessioc@alessio-laptop>
## Created: 2024-11-15
*/

#include <octave/oct.h>
#include <octave/ov-struct.h>
#include <octave/parse.h>
#include "aria_uwb_toolbox.h"

enum RCS_TYPE
{
    RCS_UNDEF,RCS_NUMBER, RCS_MATRIX, RCS_NUMBER_FREQ, RCS_MATRIX_FREQ
};

DEFUN_DLD(antenna_calc_signal_rx, args, , "-*- texinfo -*-\n\
@deftypefn {} {@var{aout} =} antenna_calc_signal_rx (@var{ant}, @var{pos}, \
@var {donoise},@var{ts} ,@var{rcs}, @var{rcs_freq})\n\
Calculate the signal received from a target or a signal transmitted from a certain point \n\
@var{ant}			is the antenna, \n\
@var{pos}			is the position where we calculated the radiated field \n\
@var{ts}			is the sampling interval for the input signal \n\
@var{do_noise}		is 1 to add thermal noise \n\
@var{rcs}			is the rcs (it can be a number or a matrix over frequency (2,2,n)) \n\
@var{rcs_freq}		is the rcs frequency support \n\
@end deftypefn")
{
	if (args.length() < 4)
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

    if ((!ant.contains("td_tx_ep"))||(!ant.contains("td_tx_et")))
    {
        error("tx signal is missing");
        return octave_value();
    }

	if (!(args(1).isreal())||((args(1).numel()!=3)&&(args(1).numel()!=0)))
    {
		error("pos must be a 3-values or empty real vector");
        return octave_value();
    }

	if (!(args(2).isreal())||(args(2).numel()!=1)||(args(2).array_value()(0)<=0))
    {
        error("ts must be a single positive real value");
    }

    int do_noise = 0;
	if (!(args(3).isreal())||(args(3).numel()!=1))
    {
        error("do noise must be 1 or 0");
        return octave_value();
    }

	do_noise = int(args(3).array_value()(0));
    if ((do_noise != 0)&&(do_noise != 1))
    {
        error("do noise must be 1 or 0");
        return octave_value();
    }

    // See what RCS we are given
    RCS_TYPE rcs_type = RCS_UNDEF;

    ComplexNDArray       rcs;
    NDArray              rcs_freqs;

	if (args.length()==4)
    {
        rcs_type = RCS_NUMBER;
        rcs.resize(dim_vector({1,1}), std::complex<double>(1.0,0.0));
    }
    else
    {
		rcs = args(4).complex_array_value();
        // 1x1
        if (rcs.numel()==1)
           rcs_type=RCS_NUMBER;

        // 1xN or Nx1
        if (((rcs.dim1()==1)||(rcs.dim2()==1))&&(rcs.numel()>1))
        {
            rcs_type=RCS_NUMBER_FREQ;
            // Check for correct frequencies
			if (args.length()!=6)
            {
                error("Frequency support for RCS is missing");
                return octave_value();
            }

			if (!(args(5).isreal())||
				((args(5).dims()(0)!=1)&&(args(5).dims()(1)!=2))||
				(args(5).numel()!=rcs.numel()))
            {
                error("RCS Freq support must be same size as RCS");
                return octave_value();
            }

			rcs_freqs = args(5).array_value();
        }

        // 2x2xN
        if ((rcs.dim1()==2)&&(rcs.dim1()==2))
        {
            if (rcs.dim3()>1)
            {
                rcs_type=RCS_MATRIX_FREQ;

                // Check for correct frequencies
				if (args.length()!=6)
                {
                    error("Frequency support for RCS is missing");
                    return octave_value();
                }

				if (!(args(5).isreal())||
					((args(5).dims()(0)!=1)&&(args(5).dims()(1)!=2))||
					(args(5).numel()!=rcs.dim3()))
                {
                    error("RCS Freq support must be same size as RCS");
                    return octave_value();
                }
            }
            else
                rcs_type = RCS_MATRIX;
        }

        if (rcs_type==RCS_UNDEF)
        {
			error("Verify RCS Data size");
            return octave_value();
        }
    }

	NDArray pos_ant = ant.getfield("position")(0).array_value();
	NDArray pos     = args(1).array_value();

    // Make same dims
    if (pos.dim1()!=pos_ant.dim1())
	{ pos.reshape(pos_ant.dims()); }

	double  ts      = args(2).array_value()(0);
	double  delay   = ant.getfield("fixed_delay")(0).array_value()(0);
	double  loss    = ant.getfield("loss")(0).array_value()(0);


    // Delay
    NDArray delta   = (pos - pos_ant);
    double  az, zen, r;

    rect_to_polar(delta(0),delta(1),delta(2), az, zen, r);

    // Check if we need to update
    bool recalc =
	   ((!ant.contains("td_aeff_t"))||
		(!ant.contains("td_aeff_p"))||
		(!ant.contains("n_ffts"))	||
		(!ant.contains("td_az"))	||
		(!ant.contains("td_zen"))	||
		(!ant.contains("td_ts"))	||
		(!ant.contains("td_delay"))	||
		(!ant.contains("td_loss")));

	ComplexNDArray td_ep;
	ComplexNDArray td_et;
	int n_samples = 0;
	if (ant.contains("td_tx_ep"))
		td_ep = ant.getfield("td_tx_ep")(0).complex_array_value();

	if (ant.contains("td_tx_ep"))
		td_et = ant.getfield("td_tx_et")(0).complex_array_value();

	n_samples = td_et.numel() > td_ep.numel() ? td_et.numel() : td_ep.numel();

	if (!recalc)
    {
        octave_value nfft = ant.getfield("n_ffts")(0);
        if (nfft.numel()!=1)
		{ recalc = true; n_samples = nfft.array_value()(0);}
        else
        {
			if ((td_ep.numel()==td_et.numel())||(nfft.array_value()(0)!=td_et.numel()))
                recalc = true;
			else
				n_samples = td_ep.numel();
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

	if (!recalc)
	{
		octave_value tsdelay = ant.getfield("td_delay")(0);
		if (tsdelay.numel()!=1)
			recalc = true;
		else
		{
			if (fabs((tsdelay.array_value()(0)-delay))>1e-15)
				recalc = true;
		}
	}

	if (!recalc)
	{
		octave_value tsloss = ant.getfield("td_loss")(0);
		if (tsloss.numel()!=1)
			recalc = true;
		else
		{
			if (fabs((tsloss.array_value()(0)-loss))>1e-15)
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
		ant = ant_build_time_domain_angle(ant_dut, ts*double(n_samples), ts, az, zen, delay, loss).map_value();
    }

    //-----------------------------------------------------------------------------------------
    // We start from the electric field radiated from the position, and we calculate power
    // flow at receiving antenna.
    // IMPORTANT NOTE: For simplicity, we assume Phi|Theta component of the electric field
    // at the target are the Phi|Theta component at Rx antenna as well.
    // Actual components mapping should be added (especially if Tx and Rx antennas are spaced
    // apart)
    //-----------------------------------------------------------------------------------------
    // When we calculate the output voltage, we move from the power domain, to the voltage domain
    // so P = V^2/(2*Z0) => V = sqrt(P * 2 * Z0)
	ComplexNDArray in_fft_ep = td_ep.fourier()*sqrt(100*ONE_OVER_ETA0)/(r*sqrt(4.0*M_PI));
	ComplexNDArray in_fft_et = td_et.fourier()*sqrt(100*ONE_OVER_ETA0)/(r*sqrt(4.0*M_PI));

    ComplexNDArray out_fft;

    ComplexNDArray aeffp_i = ant.getfield("td_aeffp")(0).complex_array_value();
    ComplexNDArray aefft_i = ant.getfield("td_aefft")(0).complex_array_value();

    out_fft.resize(in_fft_ep.dims());

    NDArray freqs_fft = ant.getfield("td_freqs")(0).array_value();
    double df = freqs_fft(1)-freqs_fft(0);

    int n_freq_of_interest = freqs_fft.numel();


    if (freqs_fft.numel() < 2)
    {
        error("Error: only one frequency available");
		return octave_value_list();
    }

    // Resample RCS if needed (that should be done once in a separate function)
    if (rcs_type == RCS_NUMBER_FREQ)
        rcs = octave::feval("interp1", octave_value_list({rcs_freqs, rcs, freqs_fft,"linear",0}))(0).complex_array_value();

    ComplexNDArray rcs_out;
    rcs_out.resize(dim_vector({2,2,n_freq_of_interest}));

    if (rcs_type == RCS_MATRIX_FREQ)
    {
        ComplexNDArray start_data;
        int n_freq_rcs = rcs_freqs.numel();

        start_data.resize(dim_vector{1,n_freq_of_interest});
        for (int p=0; p<2; p++)
            for (int t=0; t<2; t++)
            {
                // Extract the start data
                for (int f=0; f < n_freq_rcs; f++)
                    start_data(f) = rcs(p,t,f);

                ComplexNDArray out = octave::feval("interp1", octave_value_list({rcs_freqs, start_data, freqs_fft,"linear",0}))(0).complex_array_value();

                for (int f=0; f < n_freq_of_interest; f++)
                    rcs_out(p,t,f) = out(f);
            }

        rcs = rcs_out;
    }

    std::complex<double> comp_delay = std::complex<double>(1.0,0.0);
    std::complex<double> dexp       = std::exp(std::complex(0.0, -M_2PI * df * (r-REF_DISTANCE)/C0));

    std::complex<double> rcs_number;
    std::complex<double> rcs_number_pp;
    std::complex<double> rcs_number_pt;
    std::complex<double> rcs_number_tp;
    std::complex<double> rcs_number_tt;

    if (rcs_type==RCS_NUMBER)
        rcs_number = rcs(0);

    if (rcs_type==RCS_MATRIX)
    {
        rcs_number_pp = rcs(0,0);
        rcs_number_pt = rcs(0,1);
        rcs_number_tp = rcs(1,0);
        rcs_number_tt = rcs(1,1);

    }

    // Let's add Thermal noise as well: AvPwr = kTBR
    double fs = 1.0/ts;
    double noise_pwr = Kb*300.15*fs*50;
    NDArray dim_t(dim_vector({1,2}));
    dim_t(0)=1;
    dim_t(1)=n_samples;

    NDArray noise;
    ComplexNDArray noise_f;
    NDArray dir = ant.getfield("td_dir_abs")(0).array_value();

    if (do_noise == 1)
    {
        noise = octave::feval("randn", octave_value_list(octave_value(dim_t)))(0).array_value()*sqrt(noise_pwr);
        noise_f = noise.fourier();
        dir /= dir.max()(0);
    }

    for (int f =0 ; f < n_freq_of_interest; f++)
    {
        if (rcs_type==RCS_NUMBER)
        {
            out_fft(f) = (in_fft_ep(f) * aeffp_i(f) + in_fft_et(f) * aefft_i(f)) * comp_delay * rcs_number;
        }
        if (rcs_type==RCS_MATRIX)
        {
            out_fft(f) = ( (in_fft_ep(f) * rcs_number_pp + in_fft_et(f) * rcs_number_pt) * aeffp_i(f) +
                           (in_fft_ep(f) * rcs_number_tp + in_fft_et(f) * rcs_number_tt) * aefft_i(f)) * comp_delay;
        }

        if (rcs_type == RCS_NUMBER_FREQ)
            out_fft(f) = (in_fft_ep(f) * aeffp_i(f) + in_fft_et(f) * aefft_i(f)) * comp_delay * rcs(f);

        if (rcs_type == RCS_MATRIX_FREQ)
        {
            out_fft(f) = ( (in_fft_ep(f) * rcs(0,0,f)    + in_fft_et(f) * rcs(0,1,f)) * aeffp_i(f) +
                           (in_fft_ep(f) * rcs(1,0,f)    + in_fft_et(f) * rcs(1,1,f)) * aefft_i(f)) * comp_delay;
        }
        if (do_noise)
        {
            out_fft(f) = out_fft(f)+(noise_f(f) * dir(f));
        }

        comp_delay *= dexp;
    }

    // Build remaining part of the signal
    int nstart = (n_samples % 2)==0 ? n_samples/2 -1 : (n_samples-1)/2;

    for (int f=n_freq_of_interest; f < n_samples; f++, nstart--)
    {
        out_fft(f) = std::conj(out_fft(nstart));
    }

    ant.assign("td_rx",octave_value(out_fft.ifourier()));

    return octave_value_list(ant);
}
