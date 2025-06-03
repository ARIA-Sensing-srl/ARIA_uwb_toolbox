% *************************************************
% ARIA Sensing srl 2024
% Confidential-reserved
% *************************************************

%strucure is algo (code u8), rhoMin, rhoStep, rhoMax, phiMin, phiStep, phiMax (units are meter and degrees)
function [ ret_code, ret_data] = sg_canvas(board, params)

global CRC_ENGINE;

%encode according to message's format
ret_data = [];
COMMAND    = uint8('m');
command_string = [COMMAND zeros(1,128)];
index=2;

if (isempty(params) == 0)
  [command_string, index] = code_int8(command_string, index, uint8(params.algo));
  [command_string, index] = code_float(command_string, index, single(params.rhoMin));
  [command_string, index] = code_float(command_string, index, single(params.rhoStep));
  [command_string, index] = code_float(command_string, index, single(params.rhoMax));
  [command_string, index] = code_float(command_string, index, single(params.phiMin));
  [command_string, index] = code_float(command_string, index, single(params.phiStep));
  [command_string, index] = code_float(command_string, index, single(params.phiMax));
endif

command_string = command_string(1:(index-1));

encoding_and_send(board, CRC_ENGINE, command_string);
pause(.01);
stream_in = read_answer_crc(board, CRC_ENGINE);

if (isempty(stream_in)==1)
    ret_code=1;
    return;
end

if (stream_in(1)==COMMAND)
    index = 2;
    #expected size is a multiple of 13 (command excluded)

    if (length(stream_in) ~= 26)
      ret_code = 1;
      return;
    endif
    [index, ret_data.algo] = get_int8(stream_in, index);
    [index, ret_data.rhoMin] = get_float(stream_in, index);
    [index, ret_data.rhoStep] = get_float(stream_in, index);
    [index, ret_data.rhoMax] = get_float(stream_in, index);
    [index, ret_data.phiMin] = get_float(stream_in, index);
    [index, ret_data.phiStep] = get_float(stream_in, index);
    [index, ret_data.phiMax] = get_float(stream_in, index);
    ret_code = 0;
else
    ret_code = 1;
end;
end

