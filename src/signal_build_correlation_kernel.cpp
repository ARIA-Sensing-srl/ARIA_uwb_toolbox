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
## @deftypefn {} {@var{signal_out} =} signal_build_correlation_kernel (@var{pulse_in}, @var{tin}, @var{fadc})\n\
## Build a the matched filter kernel for the @var{pulse_in} signal \n\
## NB The output signal is a reversed copy of the input signal, so that we can further process it with conv \n\
## @var{pulse_in}      is the input signal \n\
## @var{tin}           is the time support for the input signal\n\
## @var{fadc}          is the frequency of the digital correlation samples \n\
## @var{signal_out}    is cross-correlation core\n\
## @var{t_extra_delay}    is the delay of xcorr operation on this kernel\n\
## @end deftypefn

## Author: Alessio Cacciatori <alessioc@alessio-laptop>
## Created: 2024-11-15
*/


#include <octave/oct.h>
#include <octave/ov-struct.h>
#include <octave/parse.h>
#include "aria_uwb_toolbox.h"


DEFUN_DLD(signal_build_correlation_kernel, args, nargout , "-*- texinfo -*-\n\
@deftypefn {} {@var{signal_out} =} signal_build_correlation_kernel (@var{pulse_in}, @var{tin}, @var{fadc})\n\
Build a the matched filter kernel for the @var{pulse_in} signal \n\
NB The output signal is a reversed copy of the input signal, so that we can further process it with conv \n\
@var{pulse_in}      is the input signal \n\
@var{tin}           is the time support for the input signal\n\
@var{fadc}          is the frequency of the digital correlation samples \n\
@var{signal_out}    is cross-correlation core\n\
@var{t_extra_delay}    is the delay of xcorr operation on this kernel\n\
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

	double t_half;
	NDArray data_in = args(0).array_value();
	double xrms = sqrt(data_in.sumsq()(0)/(double(data_in.numel())));
	int n_min = -1;
	int n_max = -1;
	double th_min = xrms/1e8;
	for (int n=0; n < data_in.numel(); n++)
	{
		double val = fabs(data_in(n));
		if (val > th_min)
		{
			if (n_min == -1)
				n_min = n;

			n_max = n;
		}
	}
	int n_length = n_max - n_min + 1;
	if (n_length % 2==0)
	{
		int n1 = (n_max+n_min-1)/2;
		int n2 = (n_max+n_min+2)/2;
		t_half = (tin(n1) + tin(n2))/2.0;
	}
	else
	{
		t_half = tin((n_max + n_min)/2);
	}
	int n_lower = ceil((t_half - tin.min()(0))*fadc);


	int nadc = n_length;
	NDArray tadc(dim_vector({1,nadc}));
	double ts = 1/fadc;
	double sampling_time = t_half - (double)n_lower * ts;
	for (int n=0; n < nadc; n++, sampling_time+=ts)
		tadc(n)=sampling_time;

/*
    int nadc = std::floor((tin.max()(0)-tin.min()(0))*fadc);
    if (nadc < 1) return octave_value();
    NDArray tadc(dim_vector({1,nadc}));

    double ts = 1/fadc;
    double sampling_time = tin.min()(0);
    for (int n=0; n < nadc; n++, sampling_time+=ts)
        tadc(n)=sampling_time;
*/
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

		for (int n=0, nstart = nmax-1; n < dlength; n++, nstart--)
            dout(n) = din(nstart);

        data_out = octave_value(dout);
    }

    if (ds_in.type==COMPLEX)
    {
		ComplexNDArray din = data_out.complex_array_value();
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

		for (int n=0, nstart = nmax-1; n < dlength; n++, nstart--)
            dout(n) = din(nstart);

        data_out = octave_value(dout);

    }
	if (nargout == 0)
		return octave_value();

	octave_value_list out(nargout);
	out(0) = data_out;
	if (nargout >=2 )
	{
		out(1) = ((double)(data_out.numel())/2.0) / fadc;
	}


	return out;

}
