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

// Perform binary search. Levels must have an odd number of elements
double quantize(double in, const NDArray& levels)
{
	int n0=0;
	int n1=levels.numel()-1;

	while (n1-n0 > 1)
	{
		int nth = (n0+n1) >> 1;
		double level = levels(nth);
		if (in >= level)
			n0 = nth;
		else
			n1 = nth;
	}
	if (n0 >= levels.numel()-1)
		n0 = levels.numel()-2;

	return levels(n0);
}

DEFUN_DLD(signal_adcconvert, args, , "-*- texinfo -*-\n\
@deftypefn {} {@var{signal_adc_out} =} signal_adcconvert (@var{rf_in}, @var{rf_signal_or_frf}, @var{fs}, @var{fmax})\n\
Down-convert the input signal with a given RF signal \n\
@var{signal_in}						is the input signal. If rf_in is a matrix, the down-conversion is performed over each row \n\
@var{time_support}					time support for the input signal \n\
@var{sampling_ck}					is the sampling clock or sampling frequency. If it is a vector, sampling instants are taken in the rising edges, at 50% between max/min\n\
@var{levels}						conversion levels\n\
@end deftypefn")
{
	if (args.length()!=4)
	{
		print_usage();
		return octave_value();
	}
	// Check input signal
	int ndims = args(0).dims().ndims();
	int nones;// = args(0).dims().num_ones();

	if ((2!=ndims))
	{
		error("signal_in must be a vector or a matrix\n");
		return octave_value();
	}

	if (args(0).dims()(1)<2)
	{
		error("signal_in must be a vector\n");
		return octave_value();
	}

	bool bMatrixSignal =  args(0).dims()(0) > 1;

	octave_idx_type npts	 = bMatrixSignal ? args(0).dims()(1) : args(0).numel();
	octave_idx_type nsignals = bMatrixSignal ? args(0).dims()(0) : 1;

	bool	bInputComplex = args(0).iscomplex();

	// Check time support
	ndims = args(1).dims().ndims();
	nones = args(1).dims().num_ones();

	if ((2!=ndims)||(nones==0)||(!args(1).isreal()))
	{
		error("time_support must be a real vector");
		return octave_value();
	}

	if (args(1).numel()!=npts)
	{
		error("time support must have the same number of points as input signal");
		return octave_value();
	}

	NDArray time_support = args(1).array_value();

	// Check clock / sampling freq
	ndims = args(2).dims().ndims();
	nones = args(2).dims().num_ones();

	if ((2!=ndims)||(nones==0)||(!args(2).isreal()))
	{
		error("sampling_ck must be a real vector or single real value");
		return octave_value();
	}

	bool bFreqSampling = args(2).numel()==1;

	if (bFreqSampling)
	{
		double fs = args(2).array_value()(0);
		if (fs <=0)
		{
			error("Sampling freq must be >0");
			return octave_value();
		}
	}
	else
	{
		if (args(2).numel()!=npts)
		{
			error("if sampling clock is a vector, it must have the same number of points as input signal");
			return octave_value();
		}
	}


	// Check levels
	ndims = args(3).dims().ndims();
	nones = args(3).dims().num_ones();

	if ((2!=ndims)||(nones!=1)||(!args(3).isreal()))
	{
		error("levels must be a real vector");
		return octave_value();
	}
	// Prepare levels so that it has an odd number of levels
	NDArray levels = args(3).array_value();
	int nlevels = levels.numel();
	if (nlevels % 2==0)
	{
//		NDArray temp = levels;
		levels.resize(dim_vector({1,nlevels+1}));
		levels(nlevels)=2.0*levels(nlevels-1)-levels(nlevels-2);
	}

	// Params check ok, continue
	NDArray sampled_time;
	// Build the ideal sampling points
	if (bFreqSampling)
	{
		double ts    = 1.0/args(2).array_value()(0);
		double t     = time_support.min()(0);
		int    ns	 = ceil((time_support.max()(0) - time_support.min()(0))/ts);
		sampled_time.resize(dim_vector({1,ns}));
		for (int i=0; i < ns; i++, t+=ts)
			sampled_time.xelem(i) = t;

	}
	else
	{
		NDArray ck = args(2).array_value();
		double th = ((ck.max()(0))+ck.min()(0))*0.5;
		int ns = npts - 1;
		int np = 0;
		for (int i=0; i < ns ; i++)
		{
			double y0 = ck(i), y1 = ck(i+1);
			if ((y0 < th)&&(y1 > th))
				np++;
		}
		if (np==0)
		{
			error("no valid ck edges");
			return octave_value();
		}
		sampled_time.resize(dim_vector({1,np}));
		int nck = 0;
		for (int i=0; i < ns ; i++)
		{
			double y0 = ck.xelem(i), y1 = ck.xelem(i+1);
			double t0 = time_support(i), t1 = time_support(i+1);
			if ((y0 < th)&&(y1 > th))
				sampled_time.xelem(nck++) = t0 + (t1-t0)*(th-y0)/(y1-y0);
		}
	}

	// Build sampled data
	octave_value sampled;
	if (bMatrixSignal)
	{
		if (bInputComplex)
		{
			std::list<octave_value> resample_in({args(1),args(0).complex_array_value().transpose(), octave_value(sampled_time) });
			sampled = octave::feval("interp1",octave_value_list(resample_in))(0).complex_array_value().transpose();
		}
		else
		{
			std::list<octave_value> resample_in({args(1),args(0).array_value().transpose(), octave_value(sampled_time) });
			sampled = octave::feval("interp1",octave_value_list(resample_in))(0).array_value().transpose();
		}
	}
	else
	{
		std::list<octave_value> resample_in({args(1),args(0), octave_value(sampled_time) });
		sampled = octave::feval("interp1",octave_value_list(resample_in))(0);
	}

	// Quantize sampled data
	octave_value out;
	if (bMatrixSignal)
	{
		npts = sampled.dims()(1);
		if (bInputComplex)
		{
			ComplexNDArray sampled_values = sampled.complex_array_value();
			for (int s=0; s < nsignals; s++)
				for (int t=0; t < npts; t++)
				{
					std::complex<double> in = sampled_values.xelem(s,t);
					sampled_values.xelem(s,t).real(quantize(in.real(),levels));
					sampled_values.xelem(s,t).imag(quantize(in.imag(),levels));
				}
			out = sampled_values;
		}
		else
		{
			NDArray sampled_values = sampled.array_value();
			for (int s=0; s < nsignals; s++)
				for (int t=0; t < npts; t++)
				{
					double in = sampled_values.xelem(t);
					sampled_values.xelem(t)= (quantize(in,levels));
				}
			out = sampled_values;
		}
	}
	else
	{
		npts = sampled.numel();
		if (bInputComplex)
		{
			ComplexNDArray sampled_values = sampled.complex_array_value();
			for (int t=0; t < npts; t++)
			{
				std::complex<double> in = sampled_values(t);
				sampled_values(t).real(quantize(in.real(),levels));
				sampled_values(t).imag(quantize(in.imag(),levels));
			}
			out = sampled_values;
		}
		else
		{
			NDArray sampled_values = sampled.array_value();
			for (int t=0; t < npts; t++)
			{
				double in = sampled_values(t);
				sampled_values(t)= (quantize(in,levels));
			}
			out = sampled_values;
		}
	}


	return out;
}
