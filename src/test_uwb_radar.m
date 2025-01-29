% Test synthetic UWB radar
% This is also a deck to perform some basic (non-extensive)
% tests on the ARIA UWB Toolbox
% Define UWB Parameter
clear variables;
close all;
clc;
pkg load aria_uwb_toolbox;
%-----------------------------
% Constants
C0 = 299792458;
%-----------------------------
% Input data

fadc= 1.792e9;% ADC Conversion frequency
div  = 4;
mult = 18;
fc = mult*fadc/div;     % Center frequency
% Hydrogen
bw = fadc/3;

%-----------------------------------------------
% Create an omnidirectional antenna
% E field is calculated with a 1W available power,
% 50 Ohm source
freqs     = fc-2*bw : 50e6 : fc+2*bw;
ant       = antenna_create(201,101,freqs);
check     = antenna_is_valid(ant);
if (!isempty(check))
  check
  return;
endif
printf("Creation and validity check --> \t ok\n");

%------------------------------------------------
% Check pwr_out.
pwr_out = antenna_radiated_power(ant);
for f=pwr_out
    if (abs(f-1)>1e-12)
      printf("Error in pwr_out detected\n");
      return;
    endif;
endfor

printf("Tx Power Check --> \t \t \t ok\n");

%------------------------------------------------
% Check pwr_out over an arbitrary range of frequencies

pwr_out_rs = antenna_radiated_power(ant,7e9:10e6:9e9);
for f=pwr_out_rs
    if (abs(f-1)>1e-8)
    printf("Error in pwr_out resampled detected\n");
    return;
  endif
endfor

printf("Creation and resampled power out --> \t ok\n");

%------------------------------------------------
% Check directivity: that should be 1 (omni)
ant = antenna_directivity(ant);
printf(" Max Directivity: \t %3.3g \n",max(max(max(ant.dir_abs))));

%------------------------------------------------
% Check group delay: this should be 3.33ns, since
% antenna's Efield is calculated at 1m sphere surface
ant = antenna_group_delay(ant);
printf(" Max Group Delay: \t %3.3g \n", max(max(max(ant.gd_t))));


%---------------------------------------------------
% Build different radar pulses
max_range = 10;
tmax      = 2* max_range / C0;
% Simulation sampling frequency
fs        = 40e9;
ts        = 1.0/fs;
prf       = 12e6;
prt       = 1/prf;
%----------------------------------
% LT103xxx
[pulse_lt103,t103] = signal_uwb_pulse("lt103", 5e-9, tmax, ts, 1, prt );
  % Normalize
  pulse_lt103 = 0.4*pulse_lt103 / max(abs(hilbert(pulse_lt103)));
%----------------------------------
% LT102
[pulse_lt102,t102] = signal_uwb_pulse("lt102", 5e-9, tmax, ts, 1, prt );
  % Normalize
  pulse_lt102 = 0.5*pulse_lt102 / max(abs(hilbert(pulse_lt102)));
%----------------------------------
% IEEE802.15.4z
code = [1 1 1 1 1 -1 -1 1 1 -1 1 -1 1];
[pulse_ieee,tieee] = signal_uwb_pulse("ieee802154z", 5e-9, tmax, ts, code, 1.0/499.2e6 );
  % Normalize
  pulse_ieee = 0.5*pulse_ieee / max(abs(hilbert(pulse_ieee)));

%----------------------------------
% Hydrogen

[pulse_hydrogen,t_hydrogen] = signal_uwb_pulse("hydrogen", 1.0/bw, tmax, ts, code, 1.0/bw);
% Normalie
  pulse_hydrogen = 0.25*pulse_hydrogen / max(abs(hilbert(pulse_hydrogen)));


%----------------------------------
% Let's compare pulses (ieee and hydrogen are given in baseband)
figure(1);
plot(t103,abs(hilbert(pulse_lt103)),'o-',...
     t102,abs(hilbert(pulse_lt102)),'x-',...
     tieee,pulse_ieee,'d-',...
     t_hydrogen,pulse_hydrogen,'s-');


% Build a carrier with some phase noise
pn_freq = [1e3 10e3 100e3 1e6 10e6 100e6];
pn_dbc  = [20 -20   -55   -80 -90  -90]+40;

carrier_hydrogen = signal_clock_phase_noise(t_hydrogen,fc, pn_freq, pn_dbc);
carrier_ieee = signal_clock_phase_noise(tieee,fc, pn_freq, pn_dbc);
pulse_rf_ieee = pulse_ieee.*carrier_ieee;
pulse_rf_hydrogen = pulse_hydrogen.*carrier_hydrogen;
figure(2);
periodogram(carrier_hydrogen,[],[],fs,'onesided');

% Compare spectra
figure(3);
periodogram(pulse_lt102,[],[],fs,'onesided');
hold on;
periodogram(pulse_lt103,[],[],fs,'onesided');
periodogram(pulse_rf_ieee,[],[],fs,'onesided');
periodogram(pulse_rf_hydrogen,[],[],fs,'onesided');
hold off;

%----------------------------------------------
% Build an array of antennas
n_tx = 4;
n_rx = 4;
lambda = C0 / fc;
array_spacing = lambda / 2.0;
antenna_array_tx = repmat(ant, n_tx);
antenna_array_rx = repmat(ant, n_rx);
antenna_array_tx(1).position = [-1.5*array_spacing, 0.0, 0.5*array_spacing];
antenna_array_tx(2).position = [-0.5*array_spacing, 0.0, 1.5*array_spacing];
antenna_array_tx(3).position = [ 0.5*array_spacing, 0.0, 1.5*array_spacing];
antenna_array_tx(4).position = [ 1.5*array_spacing, 0.0, 0.5*array_spacing];
##
antenna_array_rx(1).position = [-1.5*array_spacing, 0.0, -0.5*array_spacing];
antenna_array_rx(2).position = [-0.5*array_spacing, 0.0, -1.5*array_spacing];
antenna_array_rx(3).position = [ 0.5*array_spacing, 0.0, -1.5*array_spacing];
antenna_array_rx(4).position = [ 1.5*array_spacing, 0.0, -0.5*array_spacing];


%-------------------------------------------
% Single Target
Target_position = [-0.2,0,5];
% Select Hydrogen source



##for t = 1:n_tx
##    antenna_array_tx(t) = antenna_calc_signal_tx(antenna_array_tx(t), ...
##                            antenna_array_tx(t).position, Target_position, pulse, ts);
##endfor;





