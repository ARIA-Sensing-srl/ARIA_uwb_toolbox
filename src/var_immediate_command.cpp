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
##@deftypefn {} {@var{signal_out} =} var_immediate_update (@var{pulse_shape},@var{code},@var{tmax},@var{ts}, @var{prt})
## Build a train of UWB pulses (baseband)
## @var{pulse_shape} is a string with one of those values \"lt102\",\"lt103\",\"ieee802154z\" or \"hydrogen\"
## @var{tp} set the length of the pulses
## @var{tmax} is a single value containing the maximum time for the time-domain signal
## @var{ts} is the time sampling of the time-domain signal
## @var{code} is a length of ternary values [-1 0 1]
## @var{prt}  is the pulse repetition time in the train
##@end deftypefn

## Author: Alessio Cacciatori <alessioc@alessio-laptop>
## Created: 2024-11-15
*/

#include <octave/oct.h>
#include <octave/ov-struct.h>
#include <octave/parse.h>
#include "aria_rdk_interface_messages.h"
#include <string>
#include <fstream>
#include <filesystem>
#include <thread>

DEFUN_DLD(var_immediate_command, args, , "-*- texinfo -*-\n\
@deftypefn {} {@var{done} =} var_immediate_update (@var{var_name})\n\
Send a message to stdout so that the @var{var_name} is updated immediately from ARIA-RDK\n\
@var{var_name}      is the string with the variable name \n\
@var{done}          1 if the message has been generated\n\
@end deftypefn")
{
    boolNDArray out(dim_vector({1,1}));
    out(0)=false;
    if (args.length()!=1)
    {
        print_usage();
        return octave_value(out);
    }

    if (!args(0).is_string())
    {
        print_usage();
        return octave_value(out);
    }

    charNDArray var_name = args(0).char_array_value();

    std::string var_name_string(var_name.data(),var_name.numel());

    if (var_name_string.empty()) return octave_value(out);

    // Create a file and write the variable name
    std::string path;
    char temp [ 4096 ];
    if ( getcwd(temp, 4096) != 0)
        path = std::string ( temp );

    std::string filename(path+"/.rdk_tmp.atp");
    std::ofstream myfile(filename);

    if (!myfile.is_open())
        return octave_value(out);

    myfile << var_name_string;

    std::string str_output;
    str_output = str_message_immediate_command + "  " + var_name_string + "," + filename + "\n";
    myfile << str_output;
    myfile.close();
    octave_stdout << str_output;

	int n=0;
	while (std::filesystem::exists(filename.c_str()))
    {
	  using namespace std::chrono_literals;
	  std::this_thread::sleep_for(10ms);
	  n++;
	  if (n==1000)
	  {
		  error("Missing response from device");
		  out(0)=false;
		  return octave_value(out);
	  }
    }

    out(0)=true;
    return octave_value(out);
}
