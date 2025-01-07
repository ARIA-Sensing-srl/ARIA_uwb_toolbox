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

octave_value interp_field(const octave_value& field_in,
                          const octave_value& azimuth_in,
                          const octave_value& zenith_in,
                          const octave_value& fstart,
                          const octave_value& azimuth,
                          const octave_value& zenith,
                          const octave_value& fend,
                          bool spline)
{
    int naz  = field_in.dims()(0);
    int nzen = field_in.dims()(1);
    int nf   = field_in.dims()(2);

    double delay = REF_DISTANCE/C0;

    NDArray freq = fstart.array_value();
    if (freq.numel()!=nf)
    {
        error("Wrong number of starting freqs");
        return octave_value();
    }

    ComplexNDArray fcomplex = field_in.complex_array_value();
    // Compensate for initial delay (e.g. emPRO)
    for (int f=0; f < nf; f++)
    {
        double current_freq = freq(f);
        std::complex ref_comp = std::exp(std::complex(0.0, M_2PI * current_freq * delay));

        for (int a = 0; a < naz; a++)
            for (int z =0; z < nzen; z++)
            {
                fcomplex(a,z,f)*=ref_comp;
            }
    }

    ComplexNDArray field_out = octave::feval("interpn", octave_value_list({azimuth_in,
                                                                           zenith_in,
                                                                           fstart,
                                                                           octave_value(fcomplex),
                                                                           azimuth,
                                                                           zenith,
                                                                           fend,
                                                                           spline? "spline":"linear",0}))(0).complex_array_value();
    // Reapply the original delay
    delay = -delay;
    nf = fend.numel();

    for (int f=0; f < nf; f++)
    {
        double current_freq = fend.array_value()(f);
        std::complex ref_comp = std::exp(std::complex(0.0, M_2PI * current_freq * delay));

        for (int a = 0; a < naz; a++)
            for (int z =0; z < nzen; z++)
            {
                field_out(a,z,f)*=ref_comp;
            }
    }

    return octave_value(field_out);
}


octave_value interp_field(const octave_value& field_in, const octave_value& fstart, const octave_value& fend, bool spline)
{
    int naz  = field_in.dims()(0);
    int nzen = field_in.dims()(1);
    int nf   = field_in.dims()(2);

    double delay = REF_DISTANCE/C0;
    NDArray lin_az;  lin_az.resize(dim_vector{1,naz});
    NDArray lin_zen; lin_zen.resize(dim_vector{1,nzen});
    for (int a=0; a < naz; a++) lin_az(a)=a;
    for (int z=0; z < nzen; z++) lin_zen(z)=z;


    NDArray freq = fstart.array_value();
    if (freq.numel()!=nf)
    {
        error("Wrong number of starting freqs");
        return octave_value();
    }

    ComplexNDArray fcomplex = field_in.complex_array_value();
    // Compensate for initial delay (e.g. emPRO)
    for (int f=0; f < nf; f++)
    {
        double current_freq = freq(f);
        std::complex ref_comp = std::exp(std::complex(0.0, M_2PI * current_freq * delay));

        for (int a = 0; a < naz; a++)
            for (int z =0; z < nzen; z++)
            {
                fcomplex(a,z,f)*=ref_comp;
            }
    }

    ComplexNDArray field_out = octave::feval("interpn", octave_value_list({lin_az, lin_zen, fstart, octave_value(fcomplex), lin_az, lin_zen, fend, spline? "spline":"linear",0}))(0).complex_array_value();
    // Reapply the original delay
    delay = -delay;
    nf = fend.numel();

    for (int f=0; f < nf; f++)
    {
        double current_freq = fend.array_value()(f);
        std::complex ref_comp = std::exp(std::complex(0.0, M_2PI * current_freq * delay));

        for (int a = 0; a < naz; a++)
            for (int z =0; z < nzen; z++)
            {
                field_out(a,z,f)*=ref_comp;
            }
    }

    return octave_value(field_out);
}
