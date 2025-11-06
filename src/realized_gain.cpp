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
## @deftypefn {} {@var{antenna_out} =} antenna_realized_gain (@var{antenna_input},@var{zin})\n\
## Calculate Realized Gain of antenna.\n\
## @var{antenna_input} is the input antenna \n\
## @var{zin} is the impedance of the source/load attached. If missing no scaling of the input voltage \n\
## @ is assumed. This means, essentially, that the same configuration for the calculation of Et,Ep  \n\
##@ is used. If provided, the antenna must have meaningful Impedance, Source data \n\
## @end deftypefn"
*/

#include <octave/oct.h>
#include <octave/ovl.h>
#include <octave/ov-struct.h>
#include <octave/parse.h>
#include "aria_uwb_toolbox.h"
octave_value realized_gain(const octave_value_list& args)
{
	ComplexNDArray z_txr; // This is the impedance of the Transceiver attached to the antenna
	if (args.length()!=1)
	{
		print_usage();
		return octave_value();
	}

	// The second optional argument is Zin which is the input impedance of the source/load attached
	// to the antenna. If it is missing,
	octave_map ant = args(0).map_value();

	NDArray az = ant.getfield("azimuth")(0).array_value();
	int naz = az.numel();
	NDArray zen= ant.getfield("zenith")(0).array_value();
	int nzen= zen.numel();
	//double dz = zen(1)-zen(0);
	//double da = az(1)-az(0);
	NDArray freq = ant.getfield("freq")(0).array_value();
	int nf		 = freq.numel();

	ComplexNDArray ep = ant.getfield("ep")(0).complex_array_value();
	ComplexNDArray et = ant.getfield("et")(0).complex_array_value();

	dim_vector dims = ep.dims();

	ComplexNDArray rlzd_gain_p(dims);
	ComplexNDArray rlzd_gain_t(dims);
	ComplexNDArray effective_gain_p(dims);
	ComplexNDArray effective_gain_t(dims);
	ComplexNDArray aeff_p(dims);
	ComplexNDArray aeff_t(dims);

	ComplexNDArray Zsource_ref	= ant.getfield("Zsource")(0).complex_array_value();
	ComplexNDArray Zant_ref		= ant.getfield("Zantenna")(0).complex_array_value();
	int nzsource  = Zsource_ref.numel();
	int nzant	  = Zant_ref.numel();

	for (int f=0; f < nf; f++)
	{
		// Assuming 1V Amplitude Voltage this is the average available power density over the solid
		// angle
		Complex zs_f	= Zsource_ref(nzsource==1 ? 0 : f);
		double  rin		= zs_f.real();
		double  available_pwr	= 1.0/(8.0 * rin);

		double one_over_pav = (4.0 * M_PI / available_pwr );			// Available power
		double area_over_av_pwr = sqrt(0.5 * ONE_OVER_ETA0 * one_over_pav); // Here 0.5 is to take into account the RMS value of the Et/Ep

		Complex zant_f = Zant_ref(nzant == 1 ? 0 : f);

		Complex vant   = zant_f / (zant_f + zs_f);

		double  pwr_in = std::norm(vant) / (2.0 * zant_f.real());
		double  one_over_pin = (4.0 * M_PI / pwr_in );			// Available power
		double  area_over_pwr_in = sqrt(0.5 * ONE_OVER_ETA0 * one_over_pin);

		double actual_f = freq(f);
		double lambda = C0 / actual_f;
		double d_to_ae = sqrt(lambda*lambda/(4.0*M_PI));


		for (int z=0; z < nzen; z++ )
		{
			for (int a=0; a < naz; a++)
			{
				std::complex<double> ep_azf = ep.xelem(a,z,f);
				std::complex<double> et_azf = et.xelem(a,z,f);
				//double area = da*dz*fabs(sin(zen(z)+dz/2));

				rlzd_gain_p.xelem(a,z,f) = (ep_azf) * area_over_av_pwr;
				rlzd_gain_t.xelem(a,z,f) = (et_azf) * area_over_av_pwr;

				Complex effg_p = (ep_azf) * area_over_pwr_in ;
				Complex effg_t = (et_azf) * area_over_pwr_in ;

				effective_gain_p(a,z,f) = effg_p;
				effective_gain_t(a,z,f) = effg_t;

				aeff_p(a,z,f) = effg_p * d_to_ae ;
				aeff_t(a,z,f) = effg_t * d_to_ae ;

			}
		}
	}
	ant.setfield("rlzd_gain_p",octave_value(rlzd_gain_p));
	ant.setfield("rlzd_gain_t",octave_value(rlzd_gain_t));
	ant.setfield("eff_gain_p",octave_value(effective_gain_p));
	ant.setfield("eff_gain_t",octave_value(effective_gain_t));
	ant.setfield("aeff_t",octave_value(aeff_t));
	ant.setfield("aeff_p",octave_value(aeff_p));

	return octave_value(ant);
}
