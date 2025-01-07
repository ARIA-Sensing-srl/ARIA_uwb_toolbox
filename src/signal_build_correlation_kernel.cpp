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
##@deftypefn {} {@var{signal_out} =} signal_uwb_pulse (@var{pulse_shape},@var{code},@var{tmax},@var{ts}, @var{prt})
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
#include "aria_uwb_toolbox.h"


DEFUN_DLD(signal_build_correlation_kernel, args, , "-*- texinfo -*-\n\
@deftypefn {} {@var{signal_out} =} signal_build_correlation_kernel (@var{pulse_in}, @var{tin}, @var{fadc})\n\
Build a the matched filter kernel for the @var{pulse_in} signal \n\
@var{pulse_in}      is the input signal \n\
@var{tin}           is the time support for the input signal\n\
@var{fadc}          is the frequency of the digital correlation samples \n\
@var{signal_out}    is cross-correlation core\n\
@end deftypefn")
{
    if (args.length()!=3)
    {
        print_usage(); return octave_value();
    }

    dt_type_size ds_in = check_data_size(args(0));
    if (ds_in.size!=VECTOR)
    {
        error("pulse_in must be a vector");
        return octave_value();
    }

    dt_type_size ds = check_data_size(args(1));
    if (ds.size!=VECTOR)
    {
        error("tin must be a real vector");
        return octave_value();
    }

    ds = check_data_size(args(2));
    if ((ds.size!=NUMBER)&&(ds.type!=REAL))
    {
        error("fadc must be a real number");
        return octave_value();
    }

    double fadc = args(2).array_value()(0);
    if (fadc <= 0)
    {
        error("fadc must be positive");
        return octave_value();
    }
    NDArray tin = args(1).array_value();


    int nadc = std::floor((tin.max()(0)-tin.min()(0))*fadc);
    if (nadc < 1) return octave_value();
    NDArray tadc(dim_vector({1,nadc}));

    double ts = 1/fadc;
    double sampling_time = tin.min()(0);
    for (int n=0; n < nadc; n++, sampling_time+=ts)
        tadc(n)=sampling_time;

    octave_value data_out = octave::feval("interp1",octave_value_list({args(1),args(0),octave_value(tadc),"linear",0}))(0);
    // Find last consecutive zero

    int nmax=-1;
    int nmin=-1;
    if (ds_in.type==REAL)
    {
        NDArray din = data_out.array_value();

        for (int n=0; n< nadc; n++)
        {
            double data = din(n);
            if (fabs(data)>1e-9)
            {
                if (nmin==-1)
                    nmin = n;
                nmax = -1;
                continue;
            }
            else
            {
                if (nmax==-1)
                    nmax = n-1;
            }
        }

        if (nmax==-1)
            nmax = din.numel()-1;
        if (nmin==-1)
            nmin = din.numel()-1;

        int dlength = nmax-nmin+1;

        NDArray dout(dim_vector({1,dlength}));

        for (int n=0, nstart = nmin; n <= dlength; n++, nstart++)
            dout(n) = din(nstart);

        data_out = octave_value(dout);

    }

    if (ds_in.type==COMPLEX)
    {
        ComplexNDArray din = data_out.array_value();
        for (int n=0; n< nadc; n++)
        {
            Complex data = din(n);
            if (fabs(data)>1e-9)
            {
                if (nmin==-1)
                    nmin = n;

                nmax = -1;
                continue;
            }
            else
            {
                if (nmax==-1)
                    nmax = n-1;
            }
        }

        if (nmax==-1)
            nmax = din.numel()-1;
        if (nmin==-1)
            nmin = din.numel()-1;

        int dlength = nmax-nmin+1;

        ComplexNDArray dout(dim_vector({1,dlength}));

        for (int n=0, nstart = nmin; n <= dlength; n++, nstart++)
            dout(n) = din(nstart);

        data_out = octave_value(dout);

    }

    return data_out;

}
