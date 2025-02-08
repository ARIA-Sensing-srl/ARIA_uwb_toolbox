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
octave_value directivity(const octave_value_list& args)
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
	ComplexNDArray ep = ant.getfield("ep")(0).complex_array_value();
	ComplexNDArray et = ant.getfield("et")(0).complex_array_value();
	NDArray az = ant.getfield("azimuth")(0).array_value();
	int naz = az.numel();
	NDArray zen= ant.getfield("zenith")(0).array_value();
	int nzen= zen.numel();
	double dz = zen(1)-zen(0);
	double da = az(1)-az(0);
	NDArray freq =ant.getfield("freq")(0).array_value();
	int nf = freq.numel();

	ComplexNDArray dp,dt,aeff_p,aeff_t;
	NDArray dir_abs;
	dim_vector dims = ep.dims();
	dp.resize(dims);
	dt.resize(dims);
	aeff_p.resize(dims);
	aeff_t.resize(dims);
	dir_abs.resize(dims);


	for (int f=0; f < nf; f++)
	{
		double pwr = 0.0;
		for (int a=0; a < naz; a++)
			for (int z=0; z < nzen; z++ )
			{
				pwr += da*dz*fabs(sin(zen.xelem(z)+dz/2))*(std::norm(ep.xelem(a,z,f)) + std::norm(et.xelem(a,z,f)))*ONE_OVER_ETA0;
			}
		pwr = pwr / (4.0*M_PI);
		double one_over_pwr = 1.0/pwr;
		double actual_f = freq(f);
		double lambda = C0 / actual_f;
		double d_to_ae = sqrt(lambda*lambda/(4.0*M_PI));

		for (int a=0; a < naz; a++)
			for (int z=0; z < nzen; z++ )
			{
				std::complex<double> ep_azf = ep.xelem(a,z,f);
				std::complex<double> et_azf = et.xelem(a,z,f);
				//double area = da*dz*fabs(sin(zen(z)+dz/2));
				//double area_sq = sqrt(area);
				dp.xelem(a,z,f) = ep_azf * sqrt(ONE_OVER_ETA0 * one_over_pwr);
				dt.xelem(a,z,f) = et_azf * sqrt(ONE_OVER_ETA0 * one_over_pwr);
				aeff_p.xelem(a,z,f) = dp.xelem(a,z,f) * d_to_ae;
				aeff_t.xelem(a,z,f) = dt.xelem(a,z,f) * d_to_ae;
				dir_abs.xelem(a,z,f) = ( (std::norm(ep_azf) + std::norm(et_azf))*one_over_pwr*ONE_OVER_ETA0);
			}
	}

	aout.assign("dp",octave_value(dp));
	aout.assign("dt",octave_value(dt));
	aout.assign("aeff_t",octave_value(aeff_t));
	aout.assign("aeff_p",octave_value(aeff_p));
	aout.assign("dir_abs",octave_value(dir_abs));

	return octave_value(aout);

}
