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
##@deftypefn {} {@var{signal_out}, @var{time}, @var{toffset} =} signal_uwb_pulse (@var{pulse_shape},@var{code},@var{tmax},@var{ts}, @var{prt}) \n
## Build a train of UWB pulses (baseband)
## @var{pulse_shape} is a string with one of those values \"lt102\",\"lt103\",\"ieee802154z\" or \"hydrogen\"
## @var{tp} set the length of the pulses
## @var{tmax} is a single value containing the maximum time for the time-domain signal \n
## @var{ts} is the time sampling of the time-domain signal \n
## @var{code} is a length of ternary values [-1 0 1] \n
## @var{prt}  is the pulse repetition time in the train \n
## @var{signal_out} is the output signal \n
## @var{time} is the time support generated so that the first pulse is centered around t=0\n
## @var{toffset} is the time offset added to time support to guarantee that first pulse is centered around t=0\n
##@end deftypefn

## Author: Alessio Cacciatori <alessioc@alessio-laptop>
## Created: 2024-11-15
*/

#include <octave/oct.h>
#include <octave/ov-struct.h>
#include <octave/parse.h>


DEFUN_DLD(signal_uwb_pulse, args, nargout, "-*- texinfo -*-\n\
@deftypefn {} {@var{signal_out}, @var{time}, @var{toffset} =} signal_uwb_pulse (@var{pulse_shape},@var{tp}, @var{tmax},@var{ts}, @var{code}, @var{prt})\n\
Build a train of UWB pulses (baseband) \n\
NB The output vector is padded with a @var{tp}-long set of zeros\n\
@var{pulse_shape} is a string with one of those values \"lt102\",\"lt103\",\"ieee802154z\" or \"hydrogen\" \n\
@var{tp} set the length of the pulses \n\
@var{tmax} is a single value containing the maximum time for the time-domain signal \n\
@var{ts} is the time sampling of the time-domain signal \n\
@var{code} is a length of ternary values [-1 0 1]\n\
@var{prt}  is the pulse repetition time in the train\n\
@var{signal_out} is the output signal \n\
@var{time} is the time support generated so that the first pulse is centered around t=0\n\
@var{toffset} is the time offset added to time support to guarantee that first pulse is centered around t=0\n\
@end deftypefn")
{

    extern double lt103_data[];
    extern double lt103_t0;
    extern double lt103_ts;
    extern int    lt103_ns;

    extern double lt102_data[];
    extern double lt102_t0;
    extern double lt102_ts;
    extern int    lt102_ns;

    extern double ieee_data[];
    extern double ieee_t0;
    extern double ieee_ts;
    extern int    ieee_ns;

	double tOffset=0.0;

    if (args.length()<4)
    {
        print_usage();
        return octave_value();
    }
    // Parameters check
    if (!args(0).is_string())
    {
        error("pulse_shape must be a string\n");
        return octave_value();
    }
    //      Pulse shape
    std::string pulse_shape = args(0).string_value();
    if ((pulse_shape!="lt102")&&(pulse_shape!="lt103")&&(pulse_shape!="ieee802154z")&&(pulse_shape!="hydrogen"))
    {
        error("invalid pulse specifier\n");
        return octave_value();
    }

    //      Tp
    if ((!(args(1).isreal())||(args(1).numel()!=1)))
    {
        error("tp must be a single real value\n");
        return octave_value();
    }

    NDArray tpa = args(1).array_value();
    if ((tpa.numel()!=1)||(tpa(0)<=0.0))
    {
        error("tp must be positive");
        return octave_value();
    }
    double tp = tpa(0);

    //      Tmax
    if ((!args(2).isreal()||(args(2).numel()!=1)))
    {
        error("tmax must be a single real value\n");
        return octave_value();
    }

    NDArray tmaxa = args(2).array_value();
    if ((tmaxa.numel()!=1)||(tmaxa(0)<=0.0))
    {
        error("tmax must be positive");
        return octave_value();
    }

    double tmax = tmaxa(0);

    //      ts
    if ((!args(3).isreal()||(args(3).numel()!=1)))
    {
        error("ts must be a single real value\n");
        return octave_value();
    }

    NDArray tsa = args(3).array_value();
    if ((tsa.numel()!=1)||(tsa(0)<=0.0))
    {
        error("ts must be positive");
        return octave_value();
    }


    double ts = tsa(0);

    //  code
    NDArray code;
    if (args.length()>4) code = args(4).array_value();

    if (code.numel()>0)
    {
        // LT102 and LT103 do not allow for coding
        if ((pulse_shape=="lt102")||(pulse_shape=="lt103"))
            code.clear();
        else
        {
            for (int p=0; p < code.numel();p++)
            {
                double pcode = code(p);
                if ((pcode != 1)&&(pcode != -1)&&(pcode != 0))
                {
                    octave_stdout << "Warning: non ternary code found";
                }
            }
        }
    }
    double prt =0;
    // check prt iff we have a code sequence
    if (code.numel()>0)
    {
        if (args.length()!=6)
        {
            print_usage();
            return octave_value();
        }

        if ((!args(5).isreal()||(args(5).numel()!=1)))
        {
            error("PRT must be a single real value, when code is provided\n");
            return octave_value();
        }

        NDArray prta = args(5).array_value();
        if ((prta.numel()!=1)||(prta(0)<=0.0))
        {
            error("PRT must be positive");
            return octave_value();
        }

        prt = prta(0);
    }
    //-----------------------------------------
    // Build time domain basic pulse
    NDArray tpulse;
    NDArray pulse;
    if (pulse_shape=="lt102")
    {
        tpulse.resize(dim_vector({1,lt102_ns}));
        pulse.resize(dim_vector({1,lt102_ns}));
		double t = tOffset = lt102_t0;
        for (int n=0; n < lt102_ns; n++, t+=lt102_ts)
        {
            tpulse(n) = t;
            pulse(n)  = lt102_data[n];
        }
    }

    if (pulse_shape=="lt103")
    {
        tpulse.resize(dim_vector({1,lt103_ns}));
        pulse.resize(dim_vector({1,lt103_ns}));
		double t = tOffset = lt103_t0;
        for (int n=0; n < lt103_ns; n++, t+=lt103_ts)
        {
            tpulse(n) = t;
            pulse(n)  = lt103_data[n];
        }
    }

    if (pulse_shape == "ieee802154z")
    {
        tpulse.resize(dim_vector({1,ieee_ns}));
        pulse.resize(dim_vector({1,ieee_ns}));
		double t = tOffset = ieee_t0;
        for (int n=0; n < ieee_ns; n++, t+=ieee_ts)
        {
            tpulse(n) = t;
            pulse(n)  = ieee_data[n];
        }
    }

    if (pulse_shape == "hydrogen")
    {
        tpulse.resize(dim_vector({1,5}));
        pulse.resize(dim_vector({1,5}));
		tOffset = -tp/2;
        tpulse(0) = -tp/2;
        tpulse(1) = -tp/4;
        tpulse(2) = 0;
        tpulse(3) = tp/4;
        tpulse(4) = tp/2;

        pulse(0) = 0;
        pulse(1) = 0.5;
        pulse(2) = 1.0;
        pulse(3) = 0.5;
        pulse(4) = 0;
    }
    // Build resulting time vector
    NDArray time_support;

    int ntime_samples = (int)floor(tmax / ts)+1;

    time_support.resize(dim_vector{1,ntime_samples});

    double t=-tp;

    for (int ti=0; ti < ntime_samples; ti++, t+=ts)
        time_support(ti) = t;


    // LT102 and LT103 do not allow for pulse coding
    if ((pulse_shape == "lt102")||(pulse_shape == "lt103"))
    {
		octave_value_list out(nargout);
		if (nargout>=2) out(1) = octave_value(time_support);
		if (nargout>=1) out(0) = (octave::feval("interp1", octave_value_list({tpulse, pulse, time_support, "linear",0}))(0));
		if (nargout>=3) out(2) = octave_value(tOffset);
        return out;
    }

    NDArray pout;
    NDArray pulse_coded;
    pout.resize(dim_vector({1,ntime_samples}),0.0);

    if (code.isempty())
    {
        code.resize(dim_vector{1,1});
        code(0)=1.0;
    }

    int code_values_length = code.numel();

    for (int c = 0; c < code_values_length; c++)
    {
        pulse_coded = octave::feval("interp1", octave_value_list({tpulse, pulse, time_support, "linear", 0}))(0).array_value();
        pout = pout + pulse_coded*code(c);
        tpulse = tpulse + prt;
    }
	octave_value_list out(nargout);
	if (nargout>=1) out(0) = octave_value(pout);
	if (nargout>=2) out(1) = octave_value(time_support);
	if (nargout>=3) out(2) = octave_value(tOffset);
    return out;
}

