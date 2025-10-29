## Copyright (C) 2025 ARIA Sensing
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
## @deftypefn {} {@var{retval} =} pre_install (@var{input1}, @var{input2})
##
## @seealso{}
## @end deftypefn

## Author: ARIA Sensing
## Created: 2025-01-08

function retval = pre_install (install_data)
    install_data

  if (! isfolder (".//src"))
      printf("src folder is missing");
  end
  cd ".//src";

  if (! isfolder(install_data.archprefix))
      mkdir(install_data.archprefix);
  end
  force_build = 1;
  exclude_build =0;
  rebuild_antenna_create =                                0 | force_build;
  rebuild_antenna_is_valid =                              0 | force_build;
  rebuild_antenna_radiated_power =                        0 | force_build;
  rebuild_antenna_directivity =                           0 | force_build;
  rebuild_antenna_rlzd_gain   =                           0 | force_build;
  rebuild_antenna_group_delay =                           0 | force_build;
  rebuild_antenna_rebuild_for_time_domain =               0 | force_build;
  rebuild_antenna_rebuild_for_time_domain_single_angle =  0 | force_build;

  rebuild_antenna_calc_signal_tx =                        0 | force_build;
  rebuild_antenna_calc_signal_rx =                        0 | force_build;
  rebuild_antenna_calc_signal_rx_los =                    0 | force_build;

  rebuild_signal_uwb_pulse =                              0 | force_build;
  rebuild_signal_clock_phase_noise =                      0 | force_build;
  rebuild_signal_build_correlation_kernel =               0 | force_build;
  rebuild_signal_downconvert =                            0 | force_build;
  rebuild_signal_adcconvert =                             0 | force_build;

  rebuild_pm_demod =                                      0 | force_build;
  rebuild_delay_map=                                      0 | force_build;
  rebuild_signal_das=                                     0 | force_build;
  rebuild_signal_fdmas=                                   1 | force_build;
if exclude_build==0

  if rebuild_antenna_create==1
      clear antenna_create
      printf("Making antenna_create...\n");
      mkoctfile antenna_create.cpp uwb_toolbox_utils.cpp
      printf("Done \n");
  endif;

  if rebuild_antenna_is_valid==1
      clear antenna_is_valid
      printf("Making antenna_is_valid...\n");
      mkoctfile antenna_is_valid.cpp
      printf("Done \n");
  endif

  if rebuild_antenna_radiated_power == 1
      clear antenna_radiated_power
      printf("Making antenna_radiated_power...\n");
      mkoctfile antenna_radiated_power.cpp uwb_toolbox_utils.cpp util_interp_fields.cpp
      printf("Done \n");
  endif;

  if rebuild_antenna_directivity == 1
      clear antenna_directivity
      printf("Making antenna_directivity...\n");
      mkoctfile antenna_directivity.cpp uwb_toolbox_utils.cpp
      printf("Done \n");
  endif

  if rebuild_antenna_rlzd_gain == 1
      clear antenna_realized_gain
      printf("Making antenna_rlzd_gain...\n");
      mkoctfile antenna_realized_gain.cpp uwb_toolbox_utils.cpp
      printf("Done \n");
  endif

  if rebuild_antenna_group_delay == 1
      clear antenna_group_delay
      printf("Making antenna_group_delay...\n");
      mkoctfile antenna_group_delay.cpp uwb_toolbox_utils.cpp util_interp_fields.cpp
      printf("Done \n");
  endif

  if rebuild_antenna_rebuild_for_time_domain == 1
      clear antenna_rebuild_for_time_domain
      printf("Making antenna_rebuild_for_time_domain...\n");
      mkoctfile antenna_rebuild_for_time_domain.cpp uwb_toolbox_utils.cpp util_interp_fields.cpp
      printf("Done \n");
  endif

  if rebuild_signal_uwb_pulse == 1
      clear signal_uwb_pulse
      printf("Making UWB kernels...\n");
      mkoctfile signal_uwb_pulse.cpp uwb_toolbox_utils.cpp util_interp_fields.cpp uwb_lt102_lt103_data.cpp
      printf("Done \n");
  endif

  if rebuild_antenna_rebuild_for_time_domain_single_angle == 1
      clear antenna_rebuild_for_time_domain_single_angle
      printf("Making antenna_rebuild_for_time_domain_single_angle...\n");
      mkoctfile antenna_rebuild_for_time_domain_single_angle.cpp uwb_toolbox_utils.cpp util_interp_fields.cpp
      printf("Done \n");
  endif

  if rebuild_antenna_calc_signal_tx==1
      clear antenna_calc_signal_tx
      printf("Making antenna tx...\n");
      mkoctfile antenna_calc_signal_tx.cpp uwb_toolbox_utils.cpp ant_build_time_domain_angle.cpp directivity.cpp
          printf("Done \n");
  endif

  if rebuild_antenna_calc_signal_rx == 1
      clear antenna_calc_signal_rx
      printf("Making antenna rx...\n");
      mkoctfile antenna_calc_signal_rx.cpp uwb_toolbox_utils.cpp ant_build_time_domain_angle.cpp directivity.cpp
      printf("Done \n");
  endif

  if rebuild_antenna_calc_signal_rx_los == 1
      clear antenna_calc_signal_rx_los
      printf("Making antenna rx los...\n");
      mkoctfile antenna_calc_signal_rx_los.cpp uwb_toolbox_utils.cpp ant_build_time_domain_angle.cpp directivity.cpp
      printf("Done \n");
  endif

  if rebuild_signal_clock_phase_noise == 1
      clear signal_clock_phase_noise
      printf("Making signal_clock_phase_noise...\n");
      mkoctfile signal_clock_phase_noise.cpp uwb_toolbox_utils.cpp
      printf("Done \n");
  endif

  if rebuild_pm_demod == 1
      clear pm_demod
      printf("Making PN Demod...\n");
      mkoctfile pm_demod.cpp uwb_toolbox_utils.cpp
      printf("Done \n");
  endif

  if rebuild_signal_build_correlation_kernel == 1
    clear signal_build_correlation_kernel
    printf("Making UWB Kernel...\n");
    mkoctfile signal_build_correlation_kernel.cpp uwb_toolbox_utils.cpp
    printf("Done \n");
  endif

  if rebuild_signal_downconvert==1
    clear signal_downconvert
    printf("Making Downconversion...\n");
    mkoctfile signal_downconvert.cpp uwb_toolbox_utils.cpp
    printf("Done \n");
  endif;


   if rebuild_signal_adcconvert==1
    clear signal_adcconvert
    printf("Making ADC Conversion...\n");
    mkoctfile signal_adcconvert.cpp uwb_toolbox_utils.cpp
    printf("Done \n");
  endif;

   if rebuild_delay_map==1
    clear build_delay_map
    printf("Making Delay Map...\n");
    mkoctfile build_delay_map.cpp uwb_toolbox_utils.cpp
    printf("Done \n");
  endif;

   if rebuild_signal_das==1
    clear signal_das
    printf("Making Delay-and-Sum...\n");
    mkoctfile signal_das.cpp uwb_toolbox_utils.cpp
    printf("Done \n");
  endif;
##
##  if rebuild_signal_fdmas==1
##    clear signal_fdmas
##    printf("Making Delay-Multiply-and-Sum...\n");
##    mkoctfile signal_fdmas.cpp uwb_toolbox_utils.cpp
##    printf("Done \n");
##  endif;
  printf("-----------------------------------\n");
  printf("ARIA Modules Control \n");
  printf("------------------------------------\n");
  clear var_immediate_inquiry
  clear var_immediate_command
  clear var_immediate_update
  mkoctfile var_immediate_inquiry.cpp aria_rdk_interface_message.cpp
  mkoctfile var_immediate_command.cpp aria_rdk_interface_message.cpp
  mkoctfile var_immediate_update.cpp aria_rdk_interface_message.cpp
  mkoctfile f16tosingle.cpp
endif

  printf("-----------------------------------\n");
  printf("-----------------------------------\n");
  printf("Done \n");
  printf("------------------------------------\n");
  printf("-----------------------------------\n");

  cd ".."

endfunction
