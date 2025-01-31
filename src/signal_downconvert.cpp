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
##@deftypefn {} {@var{signal_bb_out} =} signal_downcovert (@var{rf_in}, @var{rf_signal_or_frf}, @var{fs}, @var{fmax})\n\
##Down-convert the input signal with a given RF signal \n\
##@var{rf_in}							is the input signal \n\
##@var{ rf_signal_or_frfin}				is the time down-conversion signal provided as complex (cos(*),sin(*)) \n\
##@var{fs}								is the sampling ferquency \n\
@var{fmax}								max downconversion frequency\n\
##@end deftypefn

## Author: Alessio Cacciatori <alessioc@alessio-laptop>
## Created: 2024-11-15
*/


#include <octave/oct.h>
#include <octave/ov-struct.h>
#include <octave/parse.h>
#include "aria_uwb_toolbox.h"


DEFUN_DLD(signal_downconvert, args, , "-*- texinfo -*-\n\
@deftypefn {} {@var{signal_bb_out} =} signal_downcovert (@var{rf_in}, @var{rf_signal_or_frf}, @var{fs}, @var{fmax})\n\
Down-convert the input signal with a given RF signal \n\
@var{rf_in}							is the input signal. If rf_in is a matrix, the down-conversion is performed over each row \n\
@var{rf_signal_or_frf}              is the time down-conversion signal provided as complex (cos(*),sin(*)) or a single value with the down-conversion frequency\n\
@var{fs}							time support for the input signal (and rf_signal in case it's not a frequecy value)\n\
@var{fmax}							max downconversion frequency\n\
@end deftypefn")
{
	if (args.length()!=4)
	{
		print_usage();
		return octave_value();
	}

	int ndims = args(0).dims().ndims();
	int nones = args(0).dims().num_ones();

	if (2!=ndims)
	{
		error("rf_in must be either a vector or a 2d-matrix\n");
		return octave_value();
	}

	bool bRFMatrix = nones == 0;

	if (!args(0).isreal())
	{
		error("rf_in must be real\n");
		return octave_value();
	}
	NDArray rf_in = args(0).array_value();

	octave_idx_type npts = bRFMatrix ? args(0).dims()(1) : args(0).numel();
	octave_idx_type nsignals = bRFMatrix ? args(0).dims()(0) : 1;

	ndims = args(1).dims().ndims();
	nones = args(1).dims().num_ones();

	if ((2!=ndims)||(nones==0))
	{
		error("rf_signal_or_frf must be a single real value or a vector");
		return octave_value();
	}

	bool bFreqGiven = nones==2;
	NDArray dc_signal_real;
	NDArray dc_signal_imag;
	double frf=0.;


	if (bFreqGiven)
	{
		if (args(1).isreal())
		{
			error("if center ferquency is given, it must be real");
			return octave_value();
		}
		else
			frf = args(1).array_value()(0);
	}
	else
	{
		if (args(1).numel() != npts)
		{
			error("down-conversion signal must contain the same number of elements as input signal");
			return octave_value();
		}

		if (args(1).iscomplex())
		{
			dc_signal_real = args(1).real().array_value();
			dc_signal_imag = args(1).imag().array_value();
		}
		else
		{
			dc_signal_real = args(1).array_value();
		}
	}

	bool bComplexIn = args(1).iscomplex();

	dt_type_size ds_in = check_data_size(args(2));

	if ((ds_in.size!=NUMBER)||(ds_in.type != REAL))
	{
		error("Sampling frequency must be a real number\n");
		return octave_value();
	}

	double fs = args(2).array_value()(0);
	double ts = 1.0/fs;
	if (fs <= 0)
	{
		error("Sampling frequency must be positive");
		return octave_value();
	}


	ds_in = check_data_size(args(3));

	if ((ds_in.size!=NUMBER)||(ds_in.type != REAL))
	{
		error("Fmax must be a real number\n");
		return octave_value();
	}

	double fmax = args(3).array_value()(0);

	if (fmax <= 0)
	{
		error("Fmax must be positive");
		return octave_value();
	}


	ComplexNDArray data_out;
	data_out.resize(dim_vector({nsignals,npts}));

	if (bFreqGiven)
	{
		double kFreq = 2.0 * M_PI * frf;
		Complex dPhase(cos(kFreq*ts),sin(-kFreq*ts));

		if (bRFMatrix)
		{
			for (octave_idx_type s = 0; s < nsignals; s++)
			{
				Complex Phase(1.0,0.0);
				for (octave_idx_type t = 0; t < npts; t++, Phase*=dPhase)
				{
					double din = rf_in(s,t);
					data_out(s,t) = Phase*din;
				}
			}
		}
		else
		{
			Complex Phase(1.0,0.0);
			for (octave_idx_type t = 0; t < npts; t++, Phase*=dPhase)
			{
				double din = rf_in(t);
				data_out(t) = Phase*din;
			}
		}
	}
	else
	{
		if (bRFMatrix)
		{
			for (octave_idx_type s = 0; s < nsignals; s++)
				for (octave_idx_type t = 0; t < npts; t++)
				{
					double din = rf_in(s,t);
					if (bComplexIn)
					{
						data_out(s,t) = Complex(dc_signal_real(s,t),dc_signal_imag(s,t))*din;
					}
					else
					{
						data_out(s,t) = Complex(dc_signal_real(s,t),0.0)*din;
					}
				}
		}
		else
		{
			for (octave_idx_type t = 0; t < npts; t++)
			{
				double din = rf_in(t);
				if (bComplexIn)
				{
					data_out(t) = Complex(dc_signal_real(t),dc_signal_imag(t))*din;
				}
				else
				{
					data_out(t) = Complex(dc_signal_real(t),0.0)*din;
				}
			}
		}
	}

	// Low-pass filter
	double df = fs / (double)(npts);
	ComplexNDArray fft=	bRFMatrix ? data_out.fourier(2) : data_out.fourier();

	double f_upper = fmax;
	double f_lower = fs - fmax;
	if (bRFMatrix)
	{
		for (int s=0; s < nsignals; s++)
		{
			double f=0;
			for (int n=0; n < npts; n++, f+=df)
			{
				if ((f > f_upper)&&(f<f_lower))
					fft(s,n) = Complex(0.0,0.0);
			}
		}
	}
	else
	{
		double f=0;
		for (int n=0; n < npts; n++, f+=df)
		{
			if ((f > f_upper)&&(f<f_lower))
				fft(n) = Complex(0.0,0.0);
		}
	}

	if (bRFMatrix)	{
		data_out = fft.ifourier(2);
	}
	else {
		data_out = fft.ifourier();
	}

	return octave_value(data_out);
}
