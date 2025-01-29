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
*/

#include <octave/oct.h>
#include <octave/ov-struct.h>
#include <octave/parse.h>
#include "aria_uwb_toolbox.h"

double unwrap(double dphase)
{
    dphase = fmod(dphase + M_PI, M_2PI);
    if (dphase < 0)
        dphase += M_2PI;
    return dphase - M_PI;
}


void rect_to_polar(double x, double y, double z, double& azimuth, double& zenith, double& r)
{
    r = std::sqrt(x*x+y*y+z*z);
    if (r<=1e-50)
    {
        azimuth = 0.0;
        zenith  = 0.0;
        return;
    }
    azimuth = atan2(y,x);

    zenith  = acos(z/r);
}

// Returns -1 if data is not real or complex
// Returns 0  if data is a vector
// Returns 1  if data is a single value
//
dt_type_size check_data_size(const octave_value& data)
{
    dt_type_size out;
	bool data_real = data.isreal() == 1;
	bool data_cplx = data.iscomplex() == 1;
    if ((!data_real) && (!data_cplx))
    {
        out.size = UNDEFINED;
		out.type = UNKNOWN;
        return out;
    }

    dim_vector vdims = data.dims();
    int ndim = data.ndims();

    if (vdims.num_ones() == ndim-1)
    {
        out.size = VECTOR;
		out.type = data_real ? REAL : data_cplx ? COMPLEX : UNKNOWN;
		return out;
    }

    if (vdims.num_ones() == ndim)
    {
        out.size = NUMBER;
		out.type = data_real ? REAL : data_cplx ? COMPLEX : UNKNOWN;
        return out;
    }

    if (ndim==2)
    {
        out.size = MATRIX_2D;
		out.type = data_real ? REAL : data_cplx ? COMPLEX : UNKNOWN;
        return out;
    }

    if (ndim==3)
    {
        out.size = MATRIX_3D;
		out.type = data_real ? REAL : data_cplx ? COMPLEX : UNKNOWN;
        return out;
    }

    out.size = MATRIX_ND;
	out.type = data_real ? REAL : data_cplx ? COMPLEX : UNKNOWN;
    return out;
}
