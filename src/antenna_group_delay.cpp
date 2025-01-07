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
## @deftypefn {} {@var{antenna_out} =} antenna_group_delay (@var{antenna_input})
## Calculate group delay (p/t polarized) of the input antenna.
## @seealso{}
## @end deftypefn

## Author: Alessio Cacciatori <alessioc@alessio-laptop>
## Created: 2024-11-15
*/

#include <octave/oct.h>
#include <octave/ov-struct.h>
#include <octave/parse.h>
#include "aria_uwb_toolbox.h"


DEFUN_DLD(antenna_group_delay, args, , "-*- texinfo -*-\n\
@deftypefn {} {@var{antenna_out} =} antenna_directivity (@var{antenna_input})\n\
Calculate group delay of antenna\n\
along the two components (Phi/Theta) \n\
@end deftypefn")
{
    octave_map aout;

    if (args.length() < 1)
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
    aout = ant;
    ComplexNDArray ep = ant.getfield("ep")(0).array_value();
    ComplexNDArray et = ant.getfield("et")(0).array_value();
    NDArray az = ant.getfield("azimuth")(0).array_value();
    int naz = az.numel();
    NDArray zen= ant.getfield("zenith")(0).array_value();
    int nzen= zen.numel();
    NDArray freq =ant.getfield("freq")(0).array_value();
    int nf = freq.numel();
    if (nf==1)
    {
        octave_stdout << "need at least two freq points";
        return octave_value();
    }
    NDArray gd_p(dim_vector({naz,nzen,nf-1}));
    NDArray gd_t(dim_vector({naz,nzen,nf-1}));
    // Calculate phase with unwrapping (first, compensate for
    double delay = REF_DISTANCE/C0;

    for (int f = 0; f < nf-1; f++)
    {
        double freq1 = freq(f+1);
        double freq0 = freq(f);
        std::complex reference_comp1 = std::exp(std::complex(0.0, M_2PI * freq1 * delay));
        std::complex reference_comp0 = std::exp(std::complex(0.0, M_2PI * freq0 * delay));
        double df = freq1-freq0;
        for (int a = 0; a < naz; a++)
            for (int z = 0; z < nzen; z++)
            {
                std::complex<double> ep1 = ep(a,z,f+1)*reference_comp1;
                std::complex<double> ep0 = ep(a,z,f)*reference_comp0;
                double d_omega = 2.0*M_PI*(df);
                double phase_p1 = std::arg(ep1);
                double phase_p0 = std::arg(ep0);

                gd_p(a,z,f) = delay+(phase_p0-phase_p1)/d_omega;

                std::complex<double> et1 = et(a,z,f+1)*reference_comp1;
                std::complex<double> et0 = et(a,z,f)*reference_comp0;

                double phase_t1 = std::arg(et1);
                double phase_t0 = std::arg(et0);

                gd_t(a,z,f) = delay+(phase_t0-phase_t1)/d_omega;
            }
    }
    aout.assign("gd_p",octave_value(gd_p));
    aout.assign("gd_t",octave_value(gd_t));

    return octave_value(aout);
}
