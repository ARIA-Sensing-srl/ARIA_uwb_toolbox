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
#include <octave/ov-struct.h>
#include "aria_uwb_toolbox.h"

DEFUN_DLD(antenna_is_valid, args, , "-*- texinfo -*-\n\
@deftypefn {} {@var{str_error} =} antenna_is_valid (@var{antenna_input})\n\
Check if the antenna_input is valid.\n\
If it is valid, it returns an empty string \n\
Otherwise it returns a string with the error \n\
@end deftypefn")
{
    charNDArray str_error;
    if (args.length() != 1)
    {
        print_usage();
        return octave_value(str_error);
    }

    octave_value ant_test = args(0);
    if (!ant_test.isstruct())
    {
        str_error = "Data must be a struct";
        return octave_value(str_error);
    }

    octave_map ant = ant_test.map_value();
    int naz  = 0;
    int nzen = 0;
    int nf   = 0;

    if (!ant.contains("azimuth"))
    {
        str_error = "Antenna must have azimuth support";
        return octave_value(str_error);
    }
    else
        naz = ant.getfield("azimuth")(0).numel();

    if (naz < 2)
    {
        str_error = "Antenna must have at least two azimuth points";
        return octave_value(str_error);
    }

    if (!ant.contains("zenith"))
    {
        str_error = "Antenna must have zenith support";
        return octave_value(str_error);
    }
    else
        nzen = ant.getfield("zenith")(0).numel();

    if (nzen < 2)
    {
        str_error = "Antenna must have at least two zenith points";
        return octave_value(str_error);
    }

    if (!ant.contains("freq"))
    {
        str_error = "Antenna must have freq support";
        return octave_value(str_error);
    }
    else
        nf = ant.getfield("freq")(0).numel();

    if (nf < 2)
    {
        str_error = "Antenna must have at least two freq point";
        return octave_value(str_error);
    }

    if (!ant.contains("ep"))
    {
        str_error = "Antenna must have ep specified";
        return octave_value(str_error);
    }

    dim_vector ep_dims = ant.getfield("ep")(0).dims();
    if (ep_dims(0)!=naz)
    {
        str_error = "Ep first dimension must be azimuth";
        return octave_value(str_error);
    }
    if (ep_dims(1)!=nzen)
    {
        str_error = "Ep 2nd dimension must be zenith";
        return octave_value(str_error);
    }

    if (ep_dims(2)!=nf)
    {
        str_error = "Ep 3rd dimension must be freq";
        return octave_value(str_error);
    }

    if (!ant.contains("et"))
    {
        str_error = "Antenna must have et specified";
        return octave_value(str_error);
    }

    dim_vector et_dims = ant.getfield("et")(0).dims();
    if (et_dims(0)!=naz)
    {
        str_error = "Et first dimension must be azimuth";
        return octave_value(str_error);
    }
    if (et_dims(1)!=nzen)
    {
        str_error = "Et 2nd dimension must be zenith";
        return octave_value(str_error);
    }
    if (et_dims(2)!=nf)
    {
        str_error = "Et 3rd dimension must be freq";
        return octave_value(str_error);
    }

	// Check for "position" field
	if (!ant.contains("position"))
	{
		str_error ="Antenna must contain position";
		return octave_value(str_error);
	}

	if (!ant.getfield("position")(0).isreal())
	{
		str_error = "Position must be a real vector";
		return octave_value(str_error);
	}

	if (ant.getfield("position")(0).numel()!=3)
	{
		str_error = "Position must be a 3 elements vector";
		return octave_value(str_error);
	}
	// Check for "fixed_delay" field
	if (!ant.contains("fixed_delay"))
	{
		str_error ="Antenna must contain fixed delay";
		return octave_value(str_error);
	}

	if ((!ant.getfield("fixed_delay")(0).isreal())||(ant.getfield("fixed_delay")(0).numel()!=1))
	{
		str_error = "Fixed delay must be a real single value";
		return octave_value(str_error);
	}
	// Check for "loss" field
	if (!ant.contains("loss"))
	{
		str_error ="Antenna must contain fixed loss";
		return octave_value(str_error);
	}

	if ((!ant.getfield("loss")(0).isreal())||(ant.getfield("loss")(0).numel()!=1))
	{
		str_error = "Loss must be a real single value";
		return octave_value(str_error);
	}

	return octave_value(str_error);
}
