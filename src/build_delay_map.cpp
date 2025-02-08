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
## @deftypefn {} {@var{str_error} =} antenna_is_valid (@var{antenna_input})
## Check if the antenna_input is a valid antenna
## If it is valid, it returns an empty value.
## Otherwise it returns a string with the error.
## @deftypefn {} {@var{map_out} =} build_das_map (@var{x}, @var{y}, @var{z}, @var{pos_tx}, @var{pos_rx})\n\
## Return the DAS radar map.\n\
## @var{x},@var{y},@var{z} are the coordinates  \n\
## @var{pos_tx} is a n x 3 matrix where n is the number of transmitter antennas \n\
## @var{pos_rx} is a n x 3 matrix where n is the number of receiver    antennas \n\
## @seealso{}
## @end deftypefn

## Author: Alessio Cacciatori <alessioc@alessio-laptop>
## Created: 2024-11-15
*/



#include <octave/oct.h>
#include <octave/ov-struct.h>
#include "aria_uwb_toolbox.h"

inline double sqr(double x) {return x*x;}

DEFUN_DLD(build_delay_map, args, nargout, "-*- texinfo -*-\n\
@deftypefn {} {@var{map_out},@var{cos_map},@var{sin_map} =} build_das_map (@var{x}, @var{y}, @var{z}, @var{frf}, @var{pos_tx}, @var{pos_rx})\n\
Return the space-to-delay map and sin/cos constant.\n\
@var{x},@var{y},@var{z} are the coordinates  \n\
@var{frf} is the RF frequency \n\
@var{pos_tx} is a n x 3 matrix where n is the number of transmitter antennas \n\
@var{pos_rx} is a n x 3 matrix where n is the number of receiver    antennas \n\
@end deftypefn")
{

	if (args.length()!=6)
	{
		print_usage();
		return octave_value();
	}
	// Check x
	bool vector = (args(0).ndims()==2) && (args(0).dims().num_ones()>=1);
	if ((!vector)||(!args(0).isreal()))
	{
		error("x must be a real vector");
		return octave_value();
	}
	int nx = args(0).numel();
	NDArray xv=args(0).array_value();

	// Check y
	vector = (args(1).ndims()==2) && (args(1).dims().num_ones()>=1);
	if ((!vector)||(!args(1).isreal()))
	{
		error("y must be a real vector");
		return octave_value();
	}

	int ny = args(1).numel();
	NDArray yv=args(1).array_value();

	// Check z
	vector = (args(2).ndims()==2) && (args(2).dims().num_ones()>=1);
	if ((!vector)||(!args(2).isreal()))
	{
		error("z must be a real vector");
		return octave_value();
	}
	int nz = args(2).numel();

	NDArray zv=args(2).array_value();
	// Check freq
	bool number = (args(3).dims().num_ones()==args(3).ndims());
	if ((!number)||(!args(3).isreal())||(args(3).array_value()(0)<=0))
	{
		error("freq must be a single positive value");
		return octave_value();
	}
	double freq = args(3).array_value()(0);

	// Check tx-pos
	if ((args(4).dims()(1)!=3)||(!args(4).isreal()))
	{
		error("tx position must be a n by 3 real matrix");
		return octave_value();
	}
	int n_tx = args(4).dims()(0);
	NDArray pos_tx = args(4).array_value();

	if ((args(5).dims()(1)!=3)||(!args(5).isreal()))
	{
		error("rx position must be a n by 3 real matrix");
		return octave_value();
	}
	NDArray pos_rx = args(5).array_value();
	int n_rx = args(5).dims()(0);

	NDArray			out_delay(dim_vector({nx,ny,nz,n_tx,n_rx}));
	ComplexNDArray  out_phase(dim_vector({nx,ny,nz,n_tx,n_rx}));

	double k = 2.0*M_PI*freq;
	for (int t = 0; t < n_tx; t++ )
	{
		Array<octave_idx_type> index(dim_vector({1,4}));
		double xt = pos_tx.xelem(t,0);
		double yt = pos_tx.xelem(t,1);
		double zt = pos_tx.xelem(t,2);
		index(3)=t;
		for (int r = 0; r < n_rx; r++ )
		{
			index(4)=r;
			double xr = pos_rx.xelem(r,0);
			double yr = pos_rx.xelem(r,1);
			double zr = pos_rx.xelem(r,2);
			for (int x=0; x < nx; x++)
			{
				index(0)=x;
				double xp = xv.xelem(x);
				for (int y=0; y < ny; y++)
				{
					index(1)= y;
					double yp = yv.xelem(y);
					for (int z=0; z < nz; z++)
					{
						double zp = zv.xelem(z);

						double d = sqrt( sqr(xp-xt) + sqr(yp-yt) + sqr(zp-zt)) +
								   sqrt( sqr(xp-xr) + sqr(yp-yr) + sqr(zp-zr));
						index(2) = z;
						double delay = d/C0;
						double phase = k*delay;
						out_delay.xelem(index) = delay;
						out_phase.xelem(index) = Complex(delay*cos(phase), delay*sin(phase));
						//out_sin(index) = delay*sin(phase);

					}
				}
			}
		}
	}
	octave_value_list out(nargout);
	if (nargout >= 1)
		out(0) = out_delay;
	if (nargout >= 2)
		out(1) = out_phase;
	return octave_value(out);
}
