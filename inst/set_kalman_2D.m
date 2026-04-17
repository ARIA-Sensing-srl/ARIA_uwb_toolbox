% *************************************************
% Aria Sensing srl 2024
% Confidential-reserved
% *************************************************

#set calman filter params

#params.meas_unc      estimated position measurement uncertainity
#params.speed_unc     expected speed uncertainity     (init P matrix)
#params.pos_unc       expected position uncertainity  (init P matrix)
#params.acc_unc       acceleration uncertainity (kalman filter gain)
#params.gateInTicks   gate time (ticks) before acquire target
#params.gateOutTicks  lost time (ticks) before release target
#params.parRunTicks   max parallel run (ticks) before track pruning)
#if param == [] request data

function [ ret_code, paramsOut ] = set_kalman_2D(board, params)

paramsOut=[];
ret_code = 0;

COMMAND    = uint8('K');
payloadLgth = 19;

command_string = [COMMAND];
index=2;

if (isempty(params) == 1)
   command_string = [command_string zeros(1,4)];
   [command_string, index] = code_int32(command_string, index, int32(-1));
else
  command_string = [command_string zeros(1,payloadLgth)];
  [command_string, index] = code_float(command_string, index, params.meas_unc);
  [command_string, index] = code_float(command_string, index, params.speed_unc);
  [command_string, index] = code_float(command_string, index, params.pos_unc);
  [command_string, index] = code_float(command_string, index, params.acc_unc);
  [command_string, index] = code_int8(command_string, index, params.gateInTicks);
  [command_string, index] = code_int8(command_string, index, params.gateOutTicks);
  [command_string, index] = code_int8(command_string, index, params.parRunTicks);
end


global CRC_ENGINE;
command_string = command_string(1:index-1);
encoding_and_send(board, CRC_ENGINE, command_string);

pause(.01);
stream_in = read_answer_crc(board, CRC_ENGINE);

if (isempty(stream_in)==1)
    ret_code=1;
    return;
end

if (stream_in(1)==COMMAND && (length(stream_in)==(payloadLgth+1)))
    index = 2;
    [index, paramsOut.meas_unc] = get_float(stream_in, index);
    [index, paramsOut.speed_unc] = get_float(stream_in, index);
    [index, paramsOut.pos_unc] = get_float(stream_in, index);
    [index, paramsOut.acc_unc] = get_float(stream_in, index);
    [index, paramsOut.gateInTicks] = get_int8(stream_in, index);
    [index, paramsOut.gateOutTicks] = get_int8(stream_in, index);
    [index, paramsOut.parRunTicks] = get_int8(stream_in, index);
else
    ret_code = 1;
end;


end

