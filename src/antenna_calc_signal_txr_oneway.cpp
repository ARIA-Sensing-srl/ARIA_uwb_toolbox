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

##-*- texinfo -*-\n\
##@deftypefn {} {@var{aout} =} antenna_calc_signal_txr (@var{ant_tx}, @var{ant_rx}, @var{pos_tx}, @var{pos_rx}, @var{signal}, @var{ts})\n\
## Calculate the signal received from a Rx antenna in @var{pos_tx}\n\
## @var{ant_tx}  is the tx antenna, \n\
## @var{ant_rx}  is the antenna, \n\
## @var{signal}  is the tx signal. Please note that input ref. impedance is 50Ohm \n\
## @var{ts}      is the sampling interval for the input signal \n\
## @var{do_noise}		is 1 to add thermal noise \n\
## @var{rcs}			is the rcs (it can be a number or a matrix over frequency (2,2,n)) \n\
## @var{rcs_freq}		is the rcs frequency support \n\
## @end deftypefn

## Author: Alessio Cacciatori <alessioc@alessio-laptop>
## Created: 2024-11-15
*/

#include <octave/oct.h>
#include <octave/ov-struct.h>
#include <octave/parse.h>
#include "aria_uwb_toolbox.h"

DEFUN_DLD(antenna_calc_signal_txr_oneway, args, nargout , "-*- texinfo -*-\n\
@deftypefn {} {@var{aout} =} antenna_calc_signal_txr (@var{ant_tx}, @var{ant_rx}, @var{signal}, @var{ts}, @var{do_noise})\n\
Calculate the signal received from a Rx antenna in @var{pos_tx}\n\
@var{ant_tx}  is the tx antenna, \n\
@var{ant_rx}  is the antenna, \n\
@var{signal}  is the tx signal. Please note that input ref. impedance is 50Ohm \n\
@var{ts}      is the sampling interval for the input signal \n\
@var{do_noise}		is 1 to add thermal noise \n\
@end deftypefn")
{
	if (args.length() != 4)
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
	octave_map ant_tx = ant_dut.map_value();

	ant_dut = args(1);

	check = octave::feval("antenna_is_valid",ant_dut)(0);
	if (!check.isempty())
	{
		octave_stdout << check.char_array_value();
		return octave_value();
	}
	octave_map ant_rx = ant_dut.map_value();

	if (args(2).numel()<2)
    {
        error("signal must contain at least two samples");
    }

	if (!(args(3).isreal())||(args(3).numel()!=1)||(args(3).array_value()(0)<=0))
    {
        error("ts must be a single positive real value");
    }
	double  ts			= args(3).array_value()(0);
	int     n_samples	= args(2).numel();

	//-----------------------------------------------------------------------------------------
	// Tx antenna
	NDArray pos_ant_tx = ant_tx.getfield("position")(0).array_value();
	NDArray pos_ant_rx = ant_rx.getfield("position")(0).array_value();

    // Make same dims
	if (pos_ant_rx.dim1()!=pos_ant_tx.dim1())
		{pos_ant_rx.reshape(pos_ant_tx.dims());}

	double  delay   = ant_tx.getfield("fixed_delay")(0).array_value()(0);
	double  loss    = ant_tx.getfield("loss")(0).array_value()(0);

    // Delay
	NDArray delta   = (pos_ant_rx - pos_ant_tx);
    double  az, zen, r;

    rect_to_polar(delta(0),delta(1),delta(2), az, zen, r);

    // Check if we need to update
    bool recalc =
	   ((!ant_tx.contains("td_freqs"))  ||
		(!ant_tx.contains("td_ep"))     ||
		(!ant_tx.contains("td_et"))     ||
		(!ant_tx.contains("td_aeffp"))  ||
		(!ant_tx.contains("td_aefft"))  ||
		(!ant_tx.contains("td_az"))     ||
		(!ant_tx.contains("td_zen"))    ||
		(!ant_tx.contains("td_tmax"))   ||
		(!ant_tx.contains("td_ts"))     ||
		(!ant_tx.contains("n_ffts"))	||
		(!ant_tx.contains("td_delay"))	||
		(!ant_tx.contains("td_loss")));

    if (!recalc)
    {

		octave_value nfft = ant_tx.getfield("n_ffts")(0);
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
		octave_value tsfield = ant_tx.getfield("td_ts")(0);
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
		octave_value tsdelay = ant_tx.getfield("td_delay")(0);
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
		octave_value tsloss = ant_tx.getfield("td_loss")(0);
		if (tsloss.numel()!=1)
			recalc = true;
		else
		{
			if (fabs((tsloss.array_value()(0)-loss))>1e-15)
				recalc = true;
		}
	}

	NDArray az_tx = ant_tx.getfield("azimuth")(0).array_value();
	NDArray zen_tx= ant_tx.getfield("zenith")(0).array_value();
	NDArray rotation = ant_tx.getfield("rotation")(0).array_value();

	NDArray angle_out = get_rotated_angles(octave_value(az_tx), octave_value(zen_tx), rotation(0), rotation(1), az, zen).array_value();
	az = angle_out(0);
	zen= angle_out(1);

	if (!recalc)
	{

		octave_value azfield = ant_tx.getfield("td_az")(0);
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

		octave_value zenfield = ant_tx.getfield("td_zen")(0);
		if (zenfield.numel()!=1)
			recalc = true;
		else
		{
			if (fabs((zenfield.array_value()(0)-zen))>1e-3)
				recalc = true;
		}
	}

	// If we need to recalc, update the interpolated data
    if (recalc)
    {
		octave_stdout << "Sampling at az : " << az << "Zenith. : " << zen << "\n";
		ant_tx = ant_build_time_domain_angle(ant_dut, ts*double(n_samples), ts, az, zen, delay, loss).map_value();
	}


	//-----------------------------------------------------------------------------------------
	// Rx antenna
	delta   = (pos_ant_tx - pos_ant_rx);
	rect_to_polar(delta(0),delta(1),delta(2), az, zen, r);
	recalc =
		((!ant_rx.contains("td_freqs"))  ||
		 (!ant_rx.contains("td_ep"))     ||
		 (!ant_rx.contains("td_et"))     ||
		 (!ant_rx.contains("td_aeffp"))  ||
		 (!ant_rx.contains("td_aefft"))  ||
		 (!ant_rx.contains("td_az"))     ||
		 (!ant_rx.contains("td_zen"))    ||
		 (!ant_rx.contains("td_tmax"))   ||
		 (!ant_rx.contains("td_ts"))     ||
		 (!ant_rx.contains("n_ffts"))	 ||
		 (!ant_rx.contains("td_delay"))	 ||
		 (!ant_rx.contains("td_zant"))	 ||
		 (!ant_rx.contains("td_zsource"))||
		 (!ant_rx.contains("td_loss")));

	if (!recalc)
	{
		octave_value nfft = ant_rx.getfield("n_ffts")(0);
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
		octave_value tsfield = ant_rx.getfield("td_ts")(0);
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
		octave_value tsdelay = ant_rx.getfield("td_delay")(0);
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
		octave_value tsloss = ant_rx.getfield("td_loss")(0);
		if (tsloss.numel()!=1)
			recalc = true;
		else
		{
			if (fabs((tsloss.array_value()(0)-loss))>1e-15)
				recalc = true;
		}
	}

	NDArray az_rx = ant_rx.getfield("azimuth")(0).array_value();
	NDArray zen_rx= ant_rx.getfield("zenith")(0).array_value();
	rotation = ant_rx.getfield("rotation")(0).array_value();

	angle_out = get_rotated_angles(octave_value(az_rx), octave_value(zen_rx), rotation(0), rotation(1), az, zen).array_value();
	az = angle_out(0);
	zen= angle_out(1);
	if (!recalc)
	{

		octave_value azfield = ant_rx.getfield("td_az")(0);
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

		octave_value zenfield = ant_rx.getfield("td_zen")(0);
		if (zenfield.numel()!=1)
			recalc = true;
		else
		{
			if (fabs((zenfield.array_value()(0)-zen))>1e-3)
				recalc = true;
		}
	}

	// If we need to recalc, update the interpolated data
	if (recalc)
	{
		octave_stdout << "Resampling Rx antenna \n";
		octave_stdout << "Sampling at az : " << az << "Zenith. : " << zen << "\n";
		ant_rx = ant_build_time_domain_angle(ant_rx, ts*double(n_samples), ts, az, zen, delay, loss).map_value();

	}


	ComplexNDArray in_fft = args(2).complex_array_value().fourier();

	ComplexNDArray out_fft_ep(in_fft.dims()), out_fft_et(in_fft.dims()), vload(in_fft.dims());
	ComplexNDArray epi = ant_tx.getfield("td_ep")(0).complex_array_value();
	ComplexNDArray eti = ant_tx.getfield("td_et")(0).complex_array_value();


	NDArray freqs_fft = ant_tx.getfield("td_freqs")(0).array_value();
    if (freqs_fft.numel() < 2)
    {
        error("Error: only one frequency available");
        return octave_value_list();
    }

    double df = freqs_fft(1)-freqs_fft(0);

    int n_freq_of_interest = freqs_fft.numel();

    std::complex<double> comp_delay = std::complex<double>(1.0,0.0);
	std::complex<double> dexp       = std::exp(std::complex(0.0, -M_PI *2.0 * df * (r-REF_DISTANCE)/C0));

	// Note: Ep and Et are already taking input and source impedances into account
	// There is no need to
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

	ant_tx.assign("td_tx_ep",octave_value(out_fft_ep.ifourier()));
	ant_tx.assign("td_tx_et",octave_value(out_fft_et.ifourier()));

	ComplexNDArray fft_out(in_fft.dims());
	ComplexNDArray aeff_p = ant_rx.getfield("td_aeffp")(0).complex_array_value();
	ComplexNDArray aeff_t = ant_rx.getfield("td_aefft")(0).complex_array_value();
	ComplexNDArray zsource_t = ant_rx.getfield("td_zsource")(0).complex_array_value();
	ComplexNDArray zant_t	 = ant_rx.getfield("td_zant")(0).complex_array_value();

	double		   free_space_loss = 1.0/(r*sqrt(2.0/ONE_OVER_ETA0));

	// Now calculate the received signal.
	// Note that Aeff * Flux(P) = Pavailable
	// This, in turn is |V|^2 / (8*Re(Zant))
	// -> Vt =
	for (int f=0; f < n_freq_of_interest; f++)
	{
		double actual_f = freqs_fft(f);
		Complex comp_factor = std::exp(std::complex(0.0, M_PI *2.0 * actual_f * (REF_DISTANCE)/C0));
		Complex zsource = zsource_t(f);
		Complex zant    = zant_t(f);
		double	rant	= zant.real();
		// 1. td_tx_ep / sqrt(2 * eta0) * aeff_p = input signal p
		// 1. td_tx_et / sqrt(2 * eta0) * aeff_t = input signal t
		Complex pavail_p =  free_space_loss * aeff_p(f) * out_fft_ep(f) * comp_factor;
		Complex pavail_t =  free_space_loss * aeff_t(f) * out_fft_et(f) * comp_factor;
		Complex vsource  = 2.0*(pavail_p + pavail_t) * sqrt(8.0 * rant);
		vload(f)	 = vsource * (zsource / (zant + zsource));
	}

	// Build remaining part of the signal
	nstart = (n_samples % 2)==0 ? n_samples/2 -1 : (n_samples-1)/2;

	for (int f=n_freq_of_interest; f < n_samples; f++, nstart--)
		vload(f) = std::conj(vload(nstart));

	NDArray vload_t = vload.ifourier();
	ant_rx.assign("td_vload", octave_value(vload_t));
	return octave_value_list({ant_tx,ant_rx});
}
