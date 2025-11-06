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
## @seealso{}
## @end deftypefn

## Author: Alessio Cacciatori <alessioc@alessio-laptop>
## Created: 2024-11-15
*/



#include <octave/oct.h>
#include <octave/ovl.h>
#include <octave/ov-struct.h>
#include <octave/parse.h>
#include "aria_uwb_toolbox.h"

DEFUN_DLD(antenna_read_touchstone, args, , "-*- texinfo -*-\n\
@deftypefn {} {@var{str_error} =} antenna_read_touchstone (@var{antenna_input}, @var{filename})\n\
Add the impedance to the input antenna, by reading Touchstoned data.\n\
@var{antenna_input } Input antenna structure \n\
@var{filename} Touchstone filename \n\
@end deftypefn")
{
	if (args.length()!=2)
	{
		print_usage();
		return octave_value();
	}

	octave_value ant_check_error = octave::feval("antenna_is_valid",args(0))(0);
	if (!ant_check_error.isempty())
	{
		octave_stdout << ant_check_error.char_array_value();
		return octave_value();
	}

	octave_map  ant = args(0).map_value();
	octave_value_list touchstone_data = octave::feval("read_touchstone", octave_value(args(1)),4);

	if (touchstone_data.length()==0)
	{
		error("Error while reading touchstone file");
		return octave_value();
	}

	char type = touchstone_data(0).char_array_value()(0);
	// Type can be z, y, s

	NDArray freq_s = touchstone_data(1).array_value();
	NDArray freq_a = ant.getfield("freq")(0).array_value();
	ComplexNDArray z_data = touchstone_data(2).complex_array_value();
	NDArray zref   = touchstone_data(3).array_value();

	// Data is port x port x freq. Resample at antenna frequency.
	// First convert to 'z'

	if (type == 's')
		z_data = octave::feval("s2z", octave_value_list({z_data,zref}))(0).complex_array_value();
	else
		if (type == 'y')
			z_data = octave::feval("y2z", octave_value_list({z_data,zref}))(0).complex_array_value();

	// Interpolate z-matrix, rather than S. This allows to create 0-matrix (0 extrapolation)
	int nports = z_data.dim1();
	NDArray np(dim_vector({1,nports}));
	for (int n=0; n < nports; n++) np(n)=n;
	z_data = octave::feval("interp3", octave_value_list({
							np,
							np,
							freq_s,
							z_data,
							np,
							np,
							freq_a,
							"linear",
							0}))(0).complex_array_value();

	ant.setfield("Zantenna",octave_value(z_data));
	ant.setfield("Zsource",octave_value(zref));

	return octave_value(ant);

}
