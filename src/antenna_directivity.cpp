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
## @deftypefn {} {@var{antenna_out} =} antenna_directivity (@var{antenna_input})
## Calculate directivity (both absolute and p/t polarized) of the input antenna.
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



DEFUN_DLD(antenna_directivity, args, , "-*- texinfo -*-\n\
@deftypefn {} {@var{antenna_out} =} antenna_directivity (@var{antenna_input})\n\
Calculate directivity of antenna.\n\
Both along the two components (Phi/Theta) \n\
and in dB \n\
@end deftypefn")
{
    return directivity(args);
}
