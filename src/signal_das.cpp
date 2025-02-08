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
## @deftypefn {} {@var{map_out} =} signal_das (@var{signals}, @var{time}, @var{f_rf}, @var{delay_map}, @var{phase_fact})\n\
## Return the DAS radar map.\n\
## @var{signals} is the I/Q downsampled data. It must be in (n_tx x n_rx x time) format\n\
## @var{time} is the time support for signals \n\
## @var{delay_map} is delay map that must be in (x * y * z) or (x * y * z * n_tx * n_rx) \n\
## @var{phase_fact} is the phase factor \n\
## @seealso{}
## @end deftypefn

## Author: Alessio Cacciatori <alessioc@alessio-laptop>
## Created: 2024-11-15
*/



#include <octave/oct.h>
#include <octave/ov-struct.h>
#include <octave/parse.h>

#undef DEBUG
struct search_return
{
	double _tmin;
	double _tmax;
	octave_idx_type _itmin;
	search_return (double tmin,double tmax, octave_idx_type itmin) : _tmin(tmin), _tmax(tmax), _itmin(itmin) {}
};

inline search_return binary_search_time(const NDArray& time_array, double time)
{
	octave_idx_type i_min = 0;
	octave_idx_type i_max = time_array.numel()-1;
	double t_max = time_array.xelem(i_max);
	double t_min = time_array.xelem(0);

	if (time > t_max) return			search_return(t_max,t_max,i_max);
	if (time < time_array.xelem(0))		search_return(t_min,t_min,-1);

	while (i_max - i_min > 1)
	{
		octave_idx_type i_half = (i_max + i_min)>>1;

		double t_half = time_array.xelem(i_half);

		if (time < t_half)
		{
			i_max = i_half;
			t_max = t_half;
		}
		else
		{
			i_min = i_half;
			t_min = t_half;
		}
	}

	return search_return(t_min, t_max, i_min);
}



DEFUN_DLD(signal_das, args, , "-*- texinfo -*-\n\
@deftypefn {} {@var{map_out} =} signal_das (@var{signals}, @var{time},  @var{delay_map}, @var{phase_fact})\n\
Return the DAS radar map.\n\
@var{signals} is the I/Q downsampled data. It must be in (time x n_tx x n_rx) format\n\
@var{time} is the time support for signals \n\
@var{delay_map} is delay map that must be in (x * y * z) or (x , y , z , [n_tx * n_rx]) \n\
@var{phase_fact} is the phase factor \n\
@end deftypefn")
{
	if (args.length()!=4)
	{
		print_usage();
		return octave_value();
	}

	ComplexNDArray iq_signals = args(0).complex_array_value();
	bool bSingleTxR = iq_signals.ndims()==2;
	octave_idx_type time_samples;
	int n_tx = 1;
	int n_rx = 1;
	if (!bSingleTxR)
	{
		n_tx = iq_signals.dim2();
		n_rx = iq_signals.dim3();
		time_samples = iq_signals.dim1();
	}
	else
		time_samples = iq_signals.numel();
#ifdef DEBUG
	octave_stdout << "N Tx:" << n_tx << "\n";
	octave_stdout << "N Rx:" << n_rx << "\n";
	octave_stdout << "N Samples:" << time_samples << "\n";
#endif
	// Check Time
	bool vector = (args(1).ndims()==2) && (args(1).dims().num_ones()>=1);
	if ((!args(1).isreal())||(!vector))
	{
		error("time must be a real vector");
		return octave_value();
	}
	NDArray time = args(1).array_value();
	if (time.numel()!=time_samples)
	{
		error("Time support must contain the same number of samples as BB I/Q signals");
		return octave_value();
	}

	// Map can be
	// (x * y * z) or
	// (x * y * z) * n_tx * n_rx
	// Check delay map
	if (!args(2).isreal())
	{
		error("delay map must be real");
		return octave_value();
	}


	if ((args(2).ndims()!=3)&&(args(2).ndims()!=5))
	{
		error("delay map must 3d or 5d matrix");
		return octave_value();
	}

	if (((args(2).ndims()==3)&&(!bSingleTxR))||
		((args(2).ndims()==5)&&(bSingleTxR)) ||
		(((args(2).ndims()==5)&&((args(2).dims()(3)!=n_tx)||(args(2).dims()(4)!=n_rx)))))
	{
#ifdef DEBUG
		octave_stdout << "Delay map ndims : "<<args(2).ndims() << "\n";
		octave_stdout << (bSingleTxR ? "Expected single \n" : "Expected multi \n");
		if (args(2).ndims()>=5)
		{
			octave_stdout << "DM Tx :" << args(2).dims()(3) << "\n";
			octave_stdout << "DM Rx :" << args(2).dims()(4) << "\n";
		}
#endif
		error("delay map dimension not consistent with BB data");
		return octave_value();
	}

	if (args(3).dims()!=args(2).dims())
	{
		error("Phase fact size not consistent with delay_map");
		return octave_value();

	}

	NDArray delay_map = args(2).array_value();

	octave_idx_type nx = delay_map.dim1();
	octave_idx_type ny = delay_map.dim2();
	octave_idx_type nz = delay_map.dim3();

	ComplexNDArray phase_fact = args(3).complex_array_value();

	NDArray out(dim_vector({nx,ny,nz}));

	if (bSingleTxR)
	{
		ComplexNDArray delay_fact;
		std::list<octave_value> in({args(1), args(0), args(2)});
		delay_fact = octave::feval("interp1",in)(0).complex_array_value();
		for (int x=0; x < nx; x++)
		{
			Array<octave_idx_type> index(dim_vector({1,3}));
			index(0)=x;
			for (int y=0; y < ny; y++)
			{
				index(1)=y;
				for (int z=0; z < nz; z++)
				{
					index(2)=z;
					Complex cin = delay_fact.xelem(index);
					Complex phase=phase_fact.xelem(index);
					out(x,y,z) = cin.real() * phase.real() + cin.imag() * phase.imag();
				}
			}
		}

		return octave_value(out);
	}

	int time_index_max = time_samples-1;
	for (int x=0; x < nx; x++)
	{
		Array<octave_idx_type> index(dim_vector({1,bSingleTxR? 3: 5}));
		index(0)=x;
		for (int y=0; y < ny; y++)
		{
			index(1)=y;
			for (int z=0; z < nz; z++)
			{
				index(2)=z;
				double out_sample = 0.0;


				for (int t=0; t < n_tx; t++)
				{
					index(3)=t;
					for (int r=0; r < n_rx; r++)
					{
						index(4)=r;
						double delay = delay_map.xelem(index);
						search_return sr = binary_search_time(time, delay);
						octave_idx_type index_delay = sr._itmin;
						Complex phase=phase_fact.xelem(index);
						Complex cin;
						if (index_delay==-1)
						{
							cin = iq_signals.xelem(0,t,r);
						}
						else
						{
							if (index_delay >= time_index_max )
							{
								cin = iq_signals(time_index_max,t,r);
							}
							else
							{
								Complex c0 = iq_signals.xelem(index_delay,t,r);
								Complex c1 = iq_signals.xelem(index_delay+1,t,r);
								double  t0 = sr._tmin;
								double  t1 = sr._tmax;
								cin = c0 + (c1-c0)*(delay-t0)/(t1-t0);

							}
						}

						out_sample += cin.real() * phase.real() + cin.imag() * phase.imag();
					}
				}
				out(x,y,z) = out_sample;
			}
		}
	}

	return octave_value(out);
}
