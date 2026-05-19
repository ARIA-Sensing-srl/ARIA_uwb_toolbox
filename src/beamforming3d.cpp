/* Copyright (C) 2026 ARIA Sensing
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
"-*- texinfo -*-\n\
## @deftypefn {Function File} {@var{outputImage}= } beamforming2d (@var{hradar}, @var{inputData}, @var{xx}, @var{yy}, @var{algorithm}, @var{force_rebuild_map})
## Beamform the radar data over a desired range
## Input
## @itemize
## @item @var{hradar}: Structure containing stream acquisition parameters
## @item @var{hradar}.CoreFrequency: ADC sampling rate
## @item @var{hradar}.txCenterFrequency: Carrier frequency
## @item @var{hradar}.FixedTxToAntennaDelays: Array with transmitter antenna delay compensation (s)
## @item @var{hradar}.FixedRxToAntennaDelays: Array with receiver antenna delay compensation (s)
## @item @var{hradar}.TxAntPosition: Array with antenna position [x0, y0 z0; ... ;xn yn zn]
## @item @var{hradar}.RxAntPosition: Array with antenna position [x0, y0 z0; ... ;xn yn zn]
## @item @var{hradar}.TxRxCycle: Array containning the active antenna matrix for each acquisition cucle, the array is (2, NumAntennas, NumCycles)
## @item @var{inputData}: stream input (every row is a single stream)
## @item @var{xx}: Reconstruction x base (downrange)
## @item @var{yy}: Reconstruction y base (cross range)
## @item @var{algorithm}: selected reconstruction algorithm "DMAS", "DAS", "DMAS_SR", "DAS_SR"
## @item @var{force_rebuild_map} : logic type: if provided and if true the beamforming indexes and coefficients are re-calculated. Note
## force_rebuild_map must be set to 1 if any of those values is changed: offsets, antenna sequence, tx or rx antenna positions,
## ADC freq sampling rate, offsets (Tx or Rx comp delays),  center frequency or any of xx,yy
## @end itemize
## Output
## @itemize
## @item @var{hradar}     : the radar structures where beamforming indexes and coefficients are stored (instead of being re-calculated at each cycle)
## The beamformed data is stored into the ""beamformed_image"" field\n\
## @end itemize
## @end deftypefn

## Author: ARIA Sensing
## Created: 2026-05-14
*/

#include <octave/oct.h>
#include <octave/ov-struct.h>
#include <octave/parse.h>
#include "aria_uwb_toolbox.h"

DEFUN_DLD(beamforming3d, args, , "-*- texinfo -*-\n\
## @deftypefn {Function File} {@var{outputImage}= } beamforming2d (@var{hradar}, @var{inputData}, @var{azimuth_domain}, @var{elevation_domain}, @var{rho_domain},  @var{algorithm}, @var{force_rebuild_map})\n\
## Beamform the radar data over a desired range\n\
## Input\n\
## @itemize\n\
## @item @var{hradar}: Structure containing stream acquisition parameters\n\
## @item @var{hradar}: CoreFrequency: ADC sampling rate\n\
## @item @var{hradar}: txCenterFrequency: Carrier frequency\n\
## @item @var{hradar}: FixedTxToAntennaDelays: Array with transmitter antenna delay compensation (s)\n\
## @item @var{hradar}: FixedRxToAntennaDelays: Array with receiver antenna delay compensation (s)\n\
## @item @var{hradar}: TxAntPosition: Array with antenna position [x0, y0 z0; ... ;xn yn zn]\n\
## @item @var{hradar}: RxAntPosition: Array with antenna position [x0, y0 z0; ... ;xn yn zn]\n\
## @item @var{hradar}: TxRxCycle: Array containning the active antenna matrix for each acquisition cucle, the array is (2, NumAntennas, NumCycles)\n\
## @item @var{inputData}: stream input (every row is a single stream)\n\
## @item @var{azimuth_domain}   : Reconstruction azimuth domain (cross-range) base\n\
## @item @var{elevation_domain} : Reconstruction elevation domain (cross-range) base (note the elevation=90,phi=0 corresponds to normal direction\n\
## @item @var{rho_dommain}  : Reconstruction rho domain (down-range) base\n\
## @item @var{algorithm}: selected reconstruction algorithm ""DMAS"", ""DAS"", ""DMAS_SR"", ""DAS_SR""\n\
## @item @var{force_rebuild_map} : logic type: if provided and if true the beamforming indexes and coefficients are re-calculated. \n\
## Note force_rebuild_map must be set to 1 if any of those values is changed: offsets, antenna sequence, tx or rx antenna positions,\n\
## ADC freq sampling rate, offsets (Tx or Rx comp delays), center frequency or any of xx,yy \n\
## @end itemize\n\
## Output\n\
## @itemize\n\
## @item @var{hradar}     : the radar structures where beamforming indexes and coefficients are stored (instead of being re-calculated at each cycle)\n\
## The beamformed data is stored into the ""beamformed_image"" field\n\
## @end itemize\n\
## @end deftypefn")
{
    if (args.length() < 6)
    {
        print_usage();
        return octave_value_list();
    }
    octave_map radar_struct = args(0).map_value();

    std::string algorithm = args(5).string_value();
    bool das_selected  = algorithm.compare("DAS")==0 || algorithm.compare("DAS_SR")==0;
    bool dmas_selected = algorithm.compare("DMAS")==0|| algorithm.compare("DMAS_SR")==0;
    bool sr_enabled    = algorithm.compare("DAS_SR")==0 || algorithm.compare("DMAS_SR")==0;

    bool build_remap   = false;

    if (args.length()==7)
    {
        if (!args(6).is_bool_scalar())
        {
            print_usage();
            return octave_value_list();
        }
        build_remap = args(6).bool_array_value()(0);
    }
    //------------------------------------------------
    // Check that all required matrices are present
    if (!build_remap)
        if (!radar_struct.contains("indexes_0")) build_remap = true;
    if (!build_remap)
        if (!radar_struct.contains("indexes_1")) build_remap = true;
    if (!build_remap)
        if (!radar_struct.contains("coeffs_0")) build_remap = true;
    if (!build_remap)
        if (!radar_struct.contains("coeffs_1")) build_remap = true;
    if (!build_remap)
        if (!radar_struct.contains("sin_cos_wp")) build_remap = true;
    //------------------------------------------------
    // Create the output image structure
    NDArray  azimuth_base    = args(2).array_value();  // Downrange
    NDArray  elevation_base  = args(3).array_value();  // Cross range
    NDArray  rho_base  = args(4).array_value();  // Cross range

    int nxx = azimuth_base.numel();
    int nyy = elevation_base.numel();
    int nzz = rho_base.numel();

    octave_idx_type numel_xyz = nxx * nyy * nzz;

    // Retrieve the number of streams
    ComplexNDArray  streams         = args(1).complex_array_value();
    int             streams_length  = streams.dims()(1); //i,: is the i-th reading
    int             streams_imax    = streams_length - 1;
    ComplexNDArray  beamformed_image(dim_vector({1,numel_xyz}),0);

    // Access and interpolation arrays
    NDArray indexes_0;
    NDArray indexes_1;
    NDArray coeffs_0;
    NDArray coeffs_1;
    ComplexNDArray sin_cos_wp;

    // The Beamforming indexes are in the form
    // indexes0(i,j,ixx,iyy), indexes1(i,j,ixx,iyy)
    // Where i,j are the indexes of Tx and Rx antennas during each cycle
    // ixx,iyy are the indexes of the output image which give the
    // corresponding vector of interest
    // p = x(ixx)*ux + y(iyy)*uy
    dim_vector      streams_dims    = streams.dims();
    int             nstreams        = streams_dims(0);
    int             numel_xyz_s     = numel_xyz * nstreams;

    if (build_remap)
    {
        // Every array is a 1D array to speed up access. The "internal" order is x,y,z,stream
        indexes_0 = NDArray(dim_vector({1,numel_xyz_s}),0);
        indexes_1 = NDArray(dim_vector({1,numel_xyz_s}),0);
        coeffs_0  = NDArray(dim_vector({1,numel_xyz_s}),0);
        coeffs_1  = NDArray(dim_vector({1,numel_xyz_s}),0);
        sin_cos_wp     = ComplexNDArray(dim_vector({1,numel_xyz_s}),0);

        // Get the excited-antennas sequence
        if (!radar_struct.contains("TxRxCycle"))
        {
            error("TxRxCycle is missing from the radar structure");
            return octave_value_list();
        }
        uint32NDArray antenna_seq = radar_struct.getfield("TxRxCycle")(0).uint32_array_value();

        int           ncycles     = antenna_seq.dims()(2);

        if (nstreams!=ncycles)
        {
            error("Number of streams must match the antenna sequence description");
            return octave_value_list();
        }

        if (!radar_struct.contains("CoreFrequency"))
        {
            error("CoreFrequency is missing from the radar structure");
            return octave_value_list();
        }
        if (!radar_struct.contains("txCenterFrequency"))
        {
            error("txCenterFrequency is missing from the radar structure");
            return octave_value_list();
        }

        float fadc     = radar_struct.getfield("CoreFrequency")(0).float_array_value()(0);
        float fcarrier = radar_struct.getfield("txCenterFrequency")(0).float_array_value()(0);

        // Get the channel combinations
        uint32NDArray   channel_combinations(dim_vector({ncycles,2}),0);
        // Only one Tx and one Rx antenna is excited at every cycle
        int ncols = antenna_seq.dims()(1);
        for (int n=0; n < ncycles; n++)
        {
            uint32_t csum_tx=0;
            uint32_t csum_rx=0;
            for (int c=0; c < ncols; c++)
            {
                csum_tx += antenna_seq(0,c,n);
                csum_rx += antenna_seq(1,c,n);
                if (uint32_t(antenna_seq(0,c,n))==1)
                    channel_combinations(n,0) = c;
                if (uint32_t(antenna_seq(1,c,n))==1)
                    channel_combinations(n,1) = c;
            }

            if ((csum_tx != 1)||(csum_rx != 1))
            {
                error("Invalid Tx Rx sequence defined");
                return octave_value_list();
            }
        }

        if (!radar_struct.contains("FixedTxToAntennaDelays"))
        {
            error("FixedTxToAntennaDelays is missing from the radar structure");
            return octave_value_list();
        }
        if (!radar_struct.contains("FixedRxToAntennaDelays"))
        {
            error("FixedRxToAntennaDelays is missing from the radar structure");
            return octave_value_list();
        }

        if (!radar_struct.contains("TxAntPosition"))
        {
            error("TxAntPosition is missing from the radar structure");
            return octave_value_list();
        }
        if (!radar_struct.contains("RxAntPosition"))
        {
            error("RxAntPosition is missing from the radar structure");
            return octave_value_list();
        }

        NDArray tx_ant_delay = radar_struct.getfield("FixedTxToAntennaDelays")(0).array_value();
        NDArray rx_ant_delay = radar_struct.getfield("FixedRxToAntennaDelays")(0).array_value();
        NDArray tx_ant_pos   = radar_struct.getfield("TxAntPosition")(0).array_value();
        NDArray rx_ant_pos   = radar_struct.getfield("RxAntPosition")(0).array_value();
        NDArray rtt_s(dim_vector({1,numel_xyz_s}));
        double k = 1.0 / C0;
        octave_idx_type index = 0;
        for (int ix = 0; ix < nxx; ix++)
            for (int iy = 0; iy < nyy; iy++)
                for (int iz = 0; iz < nzz; iz++)
                {
                    double r = rho_base(iz);
                    double a = azimuth_base(ix);
                    double e = elevation_base(iy);
                    double xx = r * sin(e) * cos(a);
                    double yy = r * sin(e) * sin(a);
                    double zz = r * cos(e);
                    for (int is = 0; is < nstreams; is++)
                    {
                        int tx_i = channel_combinations(is,0);
                        int rx_j = channel_combinations(is,1);
                        double rtt_nominal      = (( sqrt(  SQR( xx - tx_ant_pos(tx_i,0) ) +
                                                            SQR( yy - tx_ant_pos(tx_i,1) ) +
                                                            SQR( zz - tx_ant_pos(tx_i,2) ) ) +
                                                    sqrt(   SQR( xx - rx_ant_pos(rx_j,0) ) +
                                                            SQR( yy - rx_ant_pos(rx_j,1) ) +
                                                            SQR( zz - rx_ant_pos(rx_j,2) ) ) ) * k);

                        double tx_antenna_delay = tx_ant_delay(tx_i);
                        double rx_antenna_delay = rx_ant_delay(rx_j);
                        double tof_comp = tx_antenna_delay + rx_antenna_delay;
                        double rtt = tof_comp + rtt_nominal;
                        double rtt_norm = rtt * fadc;

                        int i0 = (int)(floor(rtt_norm));
                        int i1 = i0 + 1;
                        double c0 = i1 - rtt_norm;
                        double c1 = rtt_norm - i0;
                        // Completely out of bound. Pad to 0
                        if (((i0 <0 )&&(i1 < 0))||
                            ((i0 >= streams_length )&&(i1 >= streams_length)))
                        {
                            c0 = 0; c1 = 0; i0 = 0; i1 = 0;
                        }
                        // i0 is out of bound. Linear interp from 0 to first sample
                        if ((i0 < 0 )&&(i1 >= 0))
                        {
                            c0 = 0; i0 = 0; i1 = 0; c1=1.0;
                        }
                        // i1 is out of bound. Linear interp from last sample to 0
                        if ((i0 < streams_length )&&(i1 >= streams_length))
                        {
                            c0 = 1.0; c1 = 0.0; i0 = streams_imax; i1 = streams_imax;
                        }
                        // Both are inside limits: already calculated
                        indexes_0(index) = i0;
                        indexes_1(index) = i1;
                        coeffs_0(index) = c0;
                        coeffs_1(index) = c1;
                        rtt_s(index) = rtt;
                        // Calculate the wave propagation constant
                        double wp_phase = M_PI *2.0 * rtt * fcarrier;
                        sin_cos_wp(index) = std::complex(cos(wp_phase),sin(wp_phase));
                        index++;
                    }
            } // for (iz)
        // here we are done with the cycle
        radar_struct.assign("coeffs_0", octave_value(coeffs_0));
        radar_struct.assign("coeffs_1", octave_value(coeffs_1));
        radar_struct.assign("indexes_0", octave_value(indexes_0));
        radar_struct.assign("indexes_1", octave_value(indexes_1));
        radar_struct.assign("sin_cos_wp", octave_value(sin_cos_wp));
    } // Mapping calculation done
    else
    { // Reuse previously calculated coeffs and indexes
        coeffs_0  = radar_struct.getfield("coeffs_0")(0).array_value();
        coeffs_1  = radar_struct.getfield("coeffs_1")(0).array_value();
        indexes_0 = radar_struct.getfield("indexes_0")(0).array_value();
        indexes_1 = radar_struct.getfield("indexes_1")(0).array_value();
        sin_cos_wp= radar_struct.getfield("sin_cos_wp")(0).complex_array_value();
    }
	
    //------------------------------------------------------------------
    // Build interpolated data according to RTTs. In every point (x,y,s), the interpolated value is
    // stored
    ComplexNDArray interp_streams(dim_vector({1,numel_xyz_s}),std::complex<double>(0.0,0.0));

    octave_idx_type source_index = 0;
    for (int ix = 0; ix < nxx; ix++)
        for (int iy = 0; iy < nyy; iy++ )
            for (int iz = 0; iz < nzz; iz++ )
            {
                for (int is = 0; is < nstreams; is++, source_index++)
                {
                    double c0 = coeffs_0(source_index);
                    double c1 = coeffs_1(source_index);
                    octave_idx_type i0_index = indexes_0(source_index);
                    octave_idx_type i1_index = indexes_1(source_index);
                    std::complex<double> y_interp = streams(is, i0_index) * c0 + streams(is, i1_index) * c1; // This is the I/Q interpolated data

                    interp_streams(source_index) = y_interp * sin_cos_wp(source_index);
                }
            }
    radar_struct.assign("interp_streams", octave_value(interp_streams));
	
    //------------------------------------------------------------------
    // Proceed with actual calculation
    octave_idx_type bf_index = 0;

    source_index = 0;

    if ((das_selected)||(dmas_selected))
    {
        double k_norm = 1.0 / (double)(nstreams); // Normalization factor
        for (int ix = 0; ix < nxx; ix++)
            for (int iy = 0; iy < nyy; iy++)
                for (int iz = 0; iz < nzz; iz++,  bf_index++)
                {
                    if (das_selected)
                    {
                        std::complex<double> out(0.0,0.0);
                        for (int is = 0; is < nstreams; is++, source_index++)
                            out += interp_streams(source_index);

                        beamformed_image(bf_index) = out * k_norm;
                        continue;
                    }


                    if (dmas_selected)
                    {
                        octave_idx_type curr_a_index = source_index;

                        std::complex<double> out(0.0,0.0);

                        for (int ia = 0; ia < nstreams-1 ; ia++, curr_a_index++)
                        {
                            std::complex<double> a = interp_streams(curr_a_index);

                            octave_idx_type curr_b_index = curr_a_index +1;

                            for (int ib = ia + 1; ib < nstreams; ib++, curr_b_index++)
                                out += a * std::conj(interp_streams(curr_b_index));
                        }

                        beamformed_image(bf_index) = out * k_norm;

                        source_index+=nstreams;
                    }

                }

        if (sr_enabled)
        {

            source_index =0;
            bf_index = 0;
            for (int ix = 0; ix < nxx; ix++)
                for (int iy = 0; iy < nyy; iy++)
                    for (int iz = 0; iz < nzz; iz++,  bf_index++)
                    {
                        std::complex<double> a = 0.0;
                        double b = 0.0;
                        for (octave_idx_type is =0; is < nstreams; is++, source_index++)
                        {
                            std::complex<double> val = interp_streams(source_index);
                            a+=val;
                            b+=std::norm(val);
                        }

                        double cf = std::norm(a) * k_norm / b;
                        beamformed_image(bf_index)*=cf;
                    }
        }

        radar_struct.assign("beamformed_image", octave_value(beamformed_image.reshape(dim_vector{nzz,nyy,nxx})));
        return octave_value_list(radar_struct);
    }



    return octave_value_list();
}
