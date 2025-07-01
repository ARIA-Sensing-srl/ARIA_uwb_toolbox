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



DEFUN_DLD(antenna_realized_gain, args, , "-*- texinfo -*-\n\
@deftypefn {} {@var{antenna_out} =} antenna_realized_gain (@var{antenna_input},@var{zin})\n\
Calculate Realized Gain of antenna.\n\
@var{antenna_input} is the input antenna \n\
@var{zin} is the impedance of the transceiver. If missing a standard 50Ohm is considered \n\
@end deftypefn")
{
	ComplexNDArray z_txr; // This is the impedance of the Transceiver attached to the antenna
	if (args.length()<1)
	{
		print_usage();
		return octave_value();
	}


	octave_map ant = args(0).map_value();

	int nf   = ant.getfield("freq")(0).numel();
	int nf_zin = 1;
	ComplexNDArray zin(dim_vector({1,nf_zin}),50);
	if (args.length()==2)
		zin = args(1).complex_array_value();
	nf_zin = zin.numel();
	if ((nf_zin!=1)&&(nf_zin!=nf))
	{
		error("zin must have 1 or nFreq points where nFreq is the number of frequency points in antenna");
		return octave_value();
	}

	NDArray az = ant.getfield("azimuth")(0).array_value();
	int naz = az.numel();
	NDArray zen= ant.getfield("zenith")(0).array_value();
	int nzen= zen.numel();
	double dz = zen(1)-zen(0);
	double da = az(1)-az(0);
	NDArray freq =ant.getfield("freq")(0).array_value();
	ComplexNDArray rlzd_gain_p(dim_vector({naz,nzen,nf}));
	ComplexNDArray rlzd_gain_t(dim_vector({naz,nzen,nf}));
	ComplexNDArray ep = ant.getfield("ep")(0).complex_array_value();
	ComplexNDArray et = ant.getfield("et")(0).complex_array_value();

	for (int f=0; f < nf; f++)
	{
		// Assuming 1V Amplitude Voltage this is the average available power density over the solid
		// angle
		double one_over_pav = (4.0 * M_PI * 8.0 * nf_zin == 1 ? zin(0).real() : zin(f).real());

		for (int z=0; z < nzen; z++ )
		{
			for (int a=0; a < naz; a++)
			{
				std::complex<double> ep_azf = ep.xelem(a,z,f);
				std::complex<double> et_azf = et.xelem(a,z,f);
				double area = da*dz*fabs(sin(zen(z)+dz/2));
				double area_over_pwr = (ONE_OVER_ETA0 * one_over_pav * area);
				rlzd_gain_p.xelem(a,z,f) = std::norm(ep_azf) * area_over_pwr;
				rlzd_gain_t.xelem(a,z,f) = std::norm(et_azf) * area_over_pwr;

			}
		}
	}
	ant.setfield("rlzd_gain_p",octave_value(rlzd_gain_p));
	ant.setfield("rlzd_gain_t",octave_value(rlzd_gain_t));

	return octave_value(ant);
}
