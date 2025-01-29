/* Copyright (C) 2024 ARIA Sensing
 *
 * */
#ifndef ARIA_UWB_TOOLBOX_H
#define ARIA_UWB_TOOLBOX_H

#include <octave/ov.h>


#define ONE_OVER_ETA0  0.0026525199
#define Kb 1.380649e-23
#ifndef M_2PI
#define M_2PI 6.2831853
#endif
#define C0 299792458
#define REF_DISTANCE 1.0
// List of accessory functions
// General
enum dt_size{UNDEFINED, EMPTY, NUMBER, VECTOR, MATRIX_2D, MATRIX_3D, MATRIX_ND};
enum dt_type{UNKNOWN, REAL, COMPLEX};

enum data_size{REAL_VECTOR , COMPLEX_VECTOR, REAL_NUMBER,  COMPLEX_NUMBER, REAL_2D_MATRIX, COMPLEX_2D_MATRIX,
               REAL_3D_MATRIX, COMPLEX_3D_MATRIX, REAL_ND_MATRIX, COMPLEX_ND_MATRIX, NOT_REAL_OR_COMPLEX};

struct dt_type_size
{
    dt_size size;
    dt_type type;
};

double unwrap(double dphase);

// Coordinates mapping
void rect_to_polar(double x, double y, double z, double& azimuth, double& zenith, double& r);
// Sampling
// List of accessory functions
octave_value interp_field(const octave_value& field_in,
                          const octave_value& fstart,
                          const octave_value& fend,
                          bool spline=false);

octave_value interp_field(const octave_value& field_in,
                          const octave_value& azimuth_in,
                          const octave_value& zenith_in,
                          const octave_value& fstart,
                          const octave_value& azimuth,
                          const octave_value& zenith,
                          const octave_value& fend,
                          bool spline=false);

octave_value directivity(const octave_value_list& args);

octave_value ant_build_time_domain_angle(const octave_value_list& args);

// Check if this is a vector
dt_type_size check_data_size(const octave_value& data);

// S-Params conversions
octave_value stoz_inner(const octave_value& smat, const octave_value& zports);

#endif // ARIA_UWB_TOOLBOX_H
