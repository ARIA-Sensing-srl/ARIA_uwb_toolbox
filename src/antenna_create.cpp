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
## @deftypefn {} {@var{retval} =} antenna_create (@var{az}, @var{zen}, @var{freqs})
##
## @seealso{}
## @end deftypefn

## Author: Alessio Cacciatori <alessioc@alessio-laptop>
## Created: 2024-11-15
*/

#include <octave/oct.h>
#include <octave/ov-struct.h>
#include "aria_uwb_toolbox.h"



DEFUN_DLD(antenna_create, args, , "-*- texinfo -*-\n\
@deftypefn {} {@var{aout} =} antenna_create (@var{nAzimuthPoints},@var{nZenithPoints},@var{freqs})\n\
Create an isotropic antenna with unitary gain.\n\
nAzimuthPoints is the number of azimuth angles, \n\
nZenithPoints is the number of zenith angles and \n\
freqs is the array of frequencies \n\
@end deftypefn")
{

    octave_scalar_map aout;
    if (args.length() != 3)
    {
        print_usage();
        return octave_value(aout);
    }

    //-------------------------------_
    // Check azimuth
    NDArray temp_array = args(0).array_value();
    if ((temp_array.numel()!=1)||(!args(0).is_real_scalar()))
    {
        error("nAzimuthPoints must be a single integer value");
        return octave_value(aout);
    }

    int naz = ceil(temp_array(0));
    if (naz < 2)
    {
        error("nAzimuthPoints must be greater than 2");
        return octave_value(aout);
    }
    //-------------------------------_
    // Check zenith
    temp_array = args(1).array_value();
    if ((temp_array.numel()!=1)||(!args(1).is_real_scalar()))
    {
        error("nZenithPoints must be a single integer value");
        return octave_value(aout);
    }

    int nzen = ceil(temp_array(0));
    if (nzen < 2)
    {
        error("nZenithPoints must be greater than 2");
        return octave_value(aout);
    }
    //-------------------------------_
    // Check freqs
    NDArray freq = args(2).array_value();
    int nf = freq.numel();

    if ((nf==0)||(!args(2).isreal()))
    {
        error("Freqs must be a real vector");
        return octave_value(aout);
    }

    NDArray azimuth(dim_vector(1, naz));
    NDArray zenith(dim_vector(1, nzen));

    double da = 2*M_PI/(double)(naz-1);
    double dz = M_PI/(double)(nzen-1);
    double az = 0;
    double zen= 0;
    for (int n=0; n < naz; n++, az+=da)
        azimuth(n) = az;
    for (int n=0; n < nzen; n++, zen+=dz)
        zenith(n) = zen;

    aout.assign("freq", octave_value(freq));
    aout.assign("azimuth", azimuth);
    aout.assign("zenith",  zenith);

    dim_vector data_dim({naz,nzen,nf});
	double delay = REF_DISTANCE/C0; // Use REF_DISTANCE convention (put - sign so we don't have to do that later)

    // We assume 1W over each frequency
    ComplexNDArray ep(data_dim);
    ComplexNDArray et(data_dim);
    // NB r = 1.0m by definition
	double kPhase =  - M_PI * 2.0 * delay;
    for (int f=0; f < nf; f++)
    {
        double current_freq = freq(f);
		std::complex et_f = std::exp(std::complex(0.0, kPhase * current_freq));
        std::complex ep_f = std::complex<double>(0.0,0.0);
        double total_power   = 0.0;
        for (int z=0; z < nzen; z++)
        {
            double dArea = da*dz*fabs(sin(zenith(z) + dz /2.0));
            for (int a=0; a < naz; a++)
            {
                // Power is equally split and we enforce only et
                ep(a,z,f) = ep_f;
                et(a,z,f) = et_f;
                total_power += dArea * ONE_OVER_ETA0;
            }
        }
        double normaliz_factor = sqrt(1.0/total_power);
        for (int a=0; a < naz; a++)
            for (int z=0; z < nzen; z++)
            {
                ep(a,z,f) *= normaliz_factor;
                et(a,z,f) *= normaliz_factor;
            }
    }

    aout.assign("ep", ep);
    aout.assign("et", et);

	// Position, delay, and loss
	NDArray position(dim_vector({3,1}),0.0);
	aout.assign("position",position);

	NDArray fix_delay(dim_vector({1,1}),0.0);
	aout.assign("fixed_delay",fix_delay);

	NDArray loss(dim_vector({1,1}),0.0);
	aout.assign("loss",loss);

    return octave_value(aout);
}
