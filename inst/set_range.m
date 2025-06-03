% *************************************************
% Cover Sistemi srl 2018
% Confidential-reserved
% *************************************************
function [ ret_code,rMin,rMax ] = set_range(board, range_min, range_max)
% Start the acquisition range (in m)
% global DEFINE_OCTAVE;
global CRC_ENGINE;
min_x = min(range_min,range_max);
max_x = max(range_min,range_max);

rMin = -1;
rMax = -1;
% START_CHAR = uint8(hex2dec('FF'));
COMMAND    = uint8('x');
% END_CHAR   = uint8(hex2dec('00'));

command_string = [  COMMAND zeros(1,16)];

index=2;
[command_string, index] = code_float(command_string, index, min_x);
% command_string(index) = END_CHAR;

command_string = command_string(1:index-1);
encoding_and_send(board, CRC_ENGINE, command_string);

% Read answer
pause(.01);
stream_in = read_answer_crc(board, CRC_ENGINE);
if (isempty(stream_in)==1)
    ret_code=1;
    return;
end

if (stream_in(1)=='x')
    [~,rMin] = get_float(stream_in,2);

else    
    ret_code = 1;
    return;
end;

COMMAND    = uint8('X');

command_string = [  COMMAND zeros(1,16)];

index=2;
[command_string, index] = code_float(command_string, index, max_x);
command_string = command_string(1:index-1);
encoding_and_send(board, CRC_ENGINE, command_string);

pause(.01);
stream_in = read_answer_crc(board, CRC_ENGINE);
if (isempty(stream_in)==1)
    ret_code=1;
    return;
end

if (stream_in(1)=='X')
    [~,rMax] = get_float(stream_in,2);
        ret_code = 0;
else    
    ret_code = 1;
    return;
end;


end

