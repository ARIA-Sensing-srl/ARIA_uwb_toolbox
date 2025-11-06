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



DEFUN_DLD(rescale_ep_et_from_input_impedance, args, , "-*- texinfo -*-\n\
@deftypefn {} {@var{antenna_out} =} rescale_ep_et_from_input_impedance (@var{antenna_input},@var{zin})\n\
Rescale fields when a different source impedance is .\n\
@var{antenna_input} is the input antenna \n\
@var{zin} is the impedance of the source/load attached. If missing no scaling of the input voltage \n\
@ is assumed. This means, essentially, that the same configuration for the calculation of Et,Ep  \n\
@ is used. If provided, the antenna must have meaningful Impedance, Source data \n\
@end deftypefn")
{
	return rescale_ep_et_from_input_impedance(args);
}

octave_value rescale_ep_et_from_input_impedance(const octave_value_list& args)
{
	ComplexNDArray z_txr; // This is the impedance of the Transceiver attached to the antenna
	if (args.length()<1)
	{
		print_usage();
		return octave_value();
	}

	if (args.length()==1) return octave_value(args(0));

	// The second argument is Zin which is the input impedance of the source/load attached
	// to the antenna.

	octave_map ant = args(0).map_value();

	int nf   = ant.getfield("freq")(0).numel();
	int nf_zin = 1;

	ComplexNDArray newZsource = args(1).complex_array_value();

	nf_zin = newZsource.numel();
	if ((nf_zin!=1)&&(nf_zin!=nf))
	{
		error("zin must have 1 or nFreq points where nFreq is the number of frequency points in antenna");
		return octave_value();
	}

	int nzs=1, nzin=1;
	ComplexNDArray Zsource_ref = ant.getfield("Zsource")(0).complex_array_value();
	nzs = Zsource_ref.numel();

	if ((nzs != 1)&&(nzs != nf))
	{
		error("Antenna source impedance has wrong number of samples.");
		return octave_value();
	}

	ComplexNDArray Zantenna = ant.getfield("Zantenna")(0).complex_array_value();
	nzin = Zantenna.numel();

	if ((nzin != 1)&&(nzin != nf))
	{
		error("Antenna input impedance has wrong number of samples.");
		return octave_value();
	}

	NDArray az = ant.getfield("azimuth")(0).array_value();
	int naz = az.numel();
	NDArray zen= ant.getfield("zenith")(0).array_value();
	int nzen= zen.numel();
	NDArray freq =ant.getfield("freq")(0).array_value();
	ComplexNDArray ep = ant.getfield("ep")(0).complex_array_value();
	ComplexNDArray et = ant.getfield("et")(0).complex_array_value();


	for (int f=0; f < nf; f++)
	{
		// Assuming 1V Amplitude Voltage this is the average available power density over the solid
		// angle
		Complex Zs2 = newZsource(nf_zin == 1 ? 0 : f);
		Complex Zs1 = Zsource_ref(nzs == 1 ? 0 : f);
		Complex Zin = Zantenna(nzin == 1 ? 0 : f);

		Complex Zfactor = (Zs1 + Zin) / (Zs2 + Zin);

		for (int z=0; z < nzen; z++ )
		{
			for (int a=0; a < naz; a++)
			{
				ep(a,z,f) *= Zfactor;
				et(a,z,f) *= Zfactor;
			}
		}
	}
	ant.setfield("ep",octave_value(ep));
	ant.setfield("et",octave_value(et));
	ant.assign("Zsource",octave_value(newZsource));

	ant = directivity(octave_value(ant)).map_value();
	ant = realized_gain(octave_value(ant)).map_value();

	return octave_value(ant);
}






