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
%-----------------------------
% Settings for Hydrogen SoC
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
freqs     = fc-3*bw : 50e6 : fc+3*bw;
ant       = antenna_create(201,101,freqs);
check     = antenna_is_valid(ant);
if (!isempty(check))
  check
  return;
endif
printf("Creation and validity check --> \t ok\n");

%------------------------------------------------
% Check pwr_out.

pwr_check = 0;
if pwr_check==1
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

pwr_out_rs = antenna_radiated_power(ant,min(freqs):10e6:max(freqs));
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

end
%---------------------------------------------------
% Build different radar pulses
max_range = 20;
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
[pulse_ieee,t_ieee] = signal_uwb_pulse("ieee802154z", 5e-9, tmax, ts, code, 1.0/499.2e6 );
  % Normalize
  pulse_ieee = 0.5*pulse_ieee / max(abs(hilbert(pulse_ieee)));

%----------------------------------
% Hydrogen

[pulse_hydrogen,t_hydrogen] = signal_uwb_pulse("hydrogen", 1.0/bw, tmax, ts, code, 1.0/bw);
% Normalize
  pulse_hydrogen = 0.25*pulse_hydrogen / max(abs(hilbert(pulse_hydrogen)));


%----------------------------------
% Let's compare pulses (ieee and hydrogen are given in baseband)
figure(1);
plot(t103*1e9,abs(hilbert(pulse_lt103)),'o-',...
     t102*1e9,abs(hilbert(pulse_lt102)),'x-',...
     t_ieee*1e9,pulse_ieee,'d-',...
     t_hydrogen*1e9,pulse_hydrogen,'s-');
title("UWB Pulses, envelops");
grid on;
xlabel("time (ns)");
ylabel("amplitude (V)");


% Build a carrier with some phase noise
pn_freq = [1e3 10e3 100e3 1e6 10e6 100e6];
pn_dbc  = [20 -20   -55   -80 -90  -90];

carrier_hydrogen  = signal_clock_phase_noise(t_hydrogen,fc, pn_freq, pn_dbc+40);
carrier_ieee      = signal_clock_phase_noise(t_ieee,499.2e6*16, pn_freq, pn_dbc);
pulse_rf_ieee     = pulse_ieee.*real(carrier_ieee);
pulse_rf_hydrogen = pulse_hydrogen.*real(carrier_hydrogen);
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

n_tx = 4;
n_rx = 4;

%-------------------------------------------
% Single Target
Target_position = [-0.2,0,12];
Target_RCS = 1;
simulate_hydrogen = 1;
simulate_ieee     = 0;
% Select Hydrogen source
if simulate_hydrogen == 1
  pulse_rf = pulse_rf_hydrogen;
  t_rf     = t_hydrogen;
  carrier  = carrier_hydrogen;
  ref_bb   = pulse_hydrogen;
endif;

% Select IEEE source
if simulate_ieee == 1
  pulse_rf = pulse_rf_ieee;
  t_rf     = t_ieee;
  carrier  = carrier_hydrogen;
  ref_bb   = pulse_ieee
endif;
%-------------------------------------------
% Calculate kernel for generated pulse
kernel = signal_build_correlation_kernel(ref_bb, t_rf, fadc);
%-------------------------------------------
% Check one single Tx antenna
AntTx = antenna_array_tx(1);
AntTx = antenna_calc_signal_tx(AntTx, Target_position, pulse_rf, ts);
et_f  = AntTx.td_et;
freqs = AntTx.td_freqs;

et_s = squeeze(AntTx.et(10,10,:));
freq_s=AntTx.freq;

figure(4);
subplot(2,1,1);
plot(freqs,abs(et_f),freq_s,abs(et_s));
grid on;
xlabel("Freq (Hz)");
ylabel("Amplitude");
title("Interpolated and Original Antenna functions");
subplot(2,1,2);
xlabel("Freq (Hz)");
ylabel("Phase (rad)");
title("Interpolated and Original Antenna functions");
plot(freqs,arg(et_f),freq_s,arg(et_s));
grid on;

figure(5);
subplot(2,1,1);
plot(t_rf*1e9,real(AntTx.td_tx_et),t_rf*1e9,real(AntTx.td_tx_ep));
xlabel("time (ns)");
ylabel("E-field (V/m)");
title("Theta/Phi Electric Field vs Time");
grid on;
%-------------------------------------------
% Now receive the target reflected signal
AntRx = antenna_array_rx(1);
AntRx.td_tx_ep = AntTx.td_tx_ep;
AntRx.td_tx_et = AntTx.td_tx_et;
DoNoise = 0;
AntRx = antenna_calc_signal_rx(AntRx, Target_position, ts, DoNoise, Target_RCS);
% Theoretical (radar range)
RxSignal = real(AntRx.td_rx);
figure(5);
subplot(2,1,2);
plot(t_rf*1e9, RxSignal);
xlabel("time (ns)");
ylabel("Rx signal (V)");
title("Received signals");
grid on;
% Theoretical link budget (assume radar centered in (0,0,0))
d = norm(Target_position);
theoretical_link_loss = 10*log10(Target_RCS * C0.^2 * max(max(max(AntTx.dir_abs)))^2 / ((4*pi)^3 * fc^2 * d^4));
simulated_link_loss = -20*log10(max(abs(hilbert(pulse_rf)))/max(abs(hilbert(RxSignal))));

printf("Simulated   Link Loss : \t %3.4g dB \n", simulated_link_loss);
printf("Theoretical Link Loss : \t %3.4g dB \n", theoretical_link_loss);

%-------------------------------------------
% 7 bits ADC
levels = [-1:2/128:1-2/128];

%-------------------------------------------
% Expand the single antenna example to the MIMO
% domain

% Build cell array
Array = cell(n_tx,n_rx);
RxRf  = cell(n_tx,n_rx);
t_adc = [];
for t = 1:n_tx
  AntTx = antenna_array_tx(t);
  AntTx = antenna_calc_signal_tx(AntTx, Target_position, pulse_rf, ts);
  for r = 1:n_rx
    AntRx = antenna_array_rx(r);
    AntRx.td_tx_ep = AntTx.td_tx_ep;
    AntRx.td_tx_et = AntTx.td_tx_et;
    AntRx = antenna_calc_signal_rx(AntRx, Target_position, ts, DoNoise, Target_RCS);
    % Down-convert. Here we have a radar so we use the same carrier
    RxRf{t,r}   = real(AntRx.td_rx);
    Array{t,r} = signal_downconvert(real(AntRx.td_rx), carrier, fs, 2e9);
    % Rescale the signal (basically ideal VGA)
    %------------------------------------
    r_array = real(Array{t,r});
    i_array = imag(Array{t,r});
    max_r = abs(max(r_array));
    min_r = abs(min(r_array));
    % Apply gain so that the data is in the range (-1,1)

    max_i = abs(max(i_array));
    min_i = abs(min(i_array));
    scale_factor = max([max_i,min_i,max_r,min_r]);
    Array{t,r} = Array{t,r} / scale_factor;
    Array{t,r} = signal_adcconvert(Array{t,r},t_rf,fadc,levels);
    Array{t,r} = conv(Array{t,r},kernel,'same');
    if (isempty(t_adc))
        t_adc = (min(t_rf):1.0/fadc:max(t_rf));% - (length(kernel)-1) / (2.0*fadc);
    end
  endfor;
endfor;



%-------------------------------------------
% Expand the single antenna example to the MIMO
% domain

figure;

for t = 1:1
    for r = 1:1
        plot(t_adc*1e9, imag(Array{t,r}),t_adc*1e9, real(Array{t,r}));
        hold on;
        plot(t_rf*1e9, RxRf{t,r}/max(abs(RxRf{t,r})));
    endfor;
endfor;
xlabel("time (ns)");
ylabel("Rx signal (V)");
title("Array Received signals");
grid on;

hold off;





