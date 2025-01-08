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
