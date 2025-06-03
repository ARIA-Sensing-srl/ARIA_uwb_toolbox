% *************************************************
% ARIA Sensing srl 2024
% Confidential-reserved
% *************************************************

%set pulse bandwidth, in MHz. Return the actual value

function [ ret_code, actual_freq ] = set_pulse_bandwidth(board, freq)

global CRC_ENGINE;
COMMAND    = uint8(12);
freq = uint16(freq);

command_string = [COMMAND zeros(1,2)];
index=2;
[command_string, index] = code_int16(command_string, index, freq);

encoding_and_send(board, CRC_ENGINE, command_string);

pause(.01);
stream_in = read_answer_crc(board, CRC_ENGINE);
if (isempty(stream_in)==1)
    ret_code=1;
    return;
end

if (stream_in(1)==COMMAND)
    [~,actual_freq] = get_int16(stream_in,2);
    ret_code = 0;
else
    ret_code = 1;
end;


end

