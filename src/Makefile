# Makefile for the ARIA UWB Toolbox package for GNU Octave
#
# Copyright (C) 2025 ARIA Sensing
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING. If not, see
# <https://www.gnu.org/licenses/>.

MKOCTFILE ?= mkoctfile
SED       ?= sed

PKG_CXXFLAGS := -Wall $(PKG_CXXFLAGS_APPEND)

OCT_COMMAND_FILES = \
  var_immediate_inquiry.oct\
  var_immediate_command.oct\
  var_immediate_update.oct \
  antenna_create.oct \
  antenna_is_valid.oct \
  antenna_radiated_power.oct \
  antenna_directivity.oct \
  antenna_group_delay.oct \
  antenna_rebuild_for_time_domain.oct \
  antenna_rebuild_for_time_domain_single_angle.oct \
  antenna_calc_signal_tx.oct \
  antenna_calc_signal_rx.oct \
  signal_uwb_pulse.oct \
  signal_clock_phase_noise.oct \
  pm_demod.oct \
  signal_build_correlation_kernel.oct 

OCT_COMMAND_SOURCES = $(patsubst %.oct,%.cpp,$(OCT_COMMAND_FILES))

all: $(OCT_COMMAND_FILES)

%.o: %.cpp
	$(MKOCTFILE) $(PKG_CXXFLAGS) -c $< -o $@

%.o: %.c
	$(MKOCTFILE) -c $< -o $@

var_immediate_inquiry.oct: var_immediate_inquiry.cpp aria_rdk_interface_message.cpp
	$(MKOCTFILE) $(PKG_CXXFLAGS) $< -o $@

var_immediate_command.oct: var_immediate_command.cpp aria_rdk_interface_message.cpp
	$(MKOCTFILE) $(PKG_CXXFLAGS) $< -o $@
	
var_immediate_update.oct: var_immediate_update.cpp aria_rdk_interface_message.cpp
	$(MKOCTFILE) $(PKG_CXXFLAGS) $< -o $@

antenna_create.oct: antenna_create.cpp uwb_toolbox_utils.cpp
	$(MKOCTFILE) $(PKG_CXXFLAGS) $< -o $@

antenna_is_valid.oct: antenna_is_valid.cpp
	$(MKOCTFILE) $(PKG_CXXFLAGS) $< -o $@

antenna_radiated_power.oct: antenna_radiated_power.cpp uwb_toolbox_utils.cpp util_interp_fields.cpp
	$(MKOCTFILE) $(PKG_CXXFLAGS) $< -o $@
	
antenna_directivity.oct: antenna_directivity.cpp uwb_toolbox_utils.cpp
	$(MKOCTFILE) $(PKG_CXXFLAGS) $< -o $@
	
antenna_group_delay.oct: antenna_group_delay.cpp uwb_toolbox_utils.cpp util_interp_fields.cpp
	$(MKOCTFILE) $(PKG_CXXFLAGS) $< -o $@

antenna_rebuild_for_time_domain.oct: antenna_rebuild_for_time_domain.cpp ant_build_time_domain_angle.cpp directivity.cpp uwb_toolbox_utils.cpp util_interp_fields.cpp
	$(MKOCTFILE) $(PKG_CXXFLAGS) $< -o $@

antenna_rebuild_for_time_domain_single_angle.oct: antenna_rebuild_for_time_domain_single_angle.cpp ant_build_time_domain_angle.cpp directivity.cpp  uwb_toolbox_utils.cpp util_interp_fields.cpp
	$(MKOCTFILE) $(PKG_CXXFLAGS) $< -o $@

antenna_calc_signal_tx.oct: antenna_calc_signal_tx.cpp ant_build_time_domain_angle.cpp directivity.cpp uwb_toolbox_utils.cpp
	$(MKOCTFILE) $(PKG_CXXFLAGS) $< -o $@
	
antenna_calc_signal_rx.oct: antenna_calc_signal_rx.cpp ant_build_time_domain_angle.cpp directivity.cpp uwb_toolbox_utils.cpp 
	$(MKOCTFILE) $(PKG_CXXFLAGS) $< -o $@

signal_uwb_pulse.oct: signal_uwb_pulse.cpp uwb_toolbox_utils.cpp util_interp_fields.cpp uwb_lt102_lt103_data.cpp
	$(MKOCTFILE) $(PKG_CXXFLAGS) $< -o $@

signal_clock_phase_noise.oct: signal_clock_phase_noise.cpp uwb_toolbox_utils.cpp
	$(MKOCTFILE) $(PKG_CXXFLAGS) $< -o $@

pm_demod.oct: pm_demod.cpp uwb_toolbox_utils.cpp
	$(MKOCTFILE) $(PKG_CXXFLAGS) $< -o $@

signal_build_correlation_kernel.oct: signal_build_correlation_kernel.cpp uwb_toolbox_utils.cpp
	$(MKOCTFILE) $(PKG_CXXFLAGS) $< -o $@

PKG_ADD PKG_DEL: $(OCT_COMMAND_SOURCES)
	$(SED) -n -e 's/.*$@: \(.*\)/\1/p' $^ > $@-t
	mv $@-t $@

clean:
	-rm -f *.o *.oct PKG_*

.PHONY: all clean
