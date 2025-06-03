% *************************************************
% ARIA Sensing srl 2024
% Confidential-reserved
% *************************************************

%set VGA gains

function [ ret_code, actual_freq ] = set_vgas_gain(board, ich, qch)

global CRC_ENGINE;
COMMAND    = uint8(14);

command_string = [COMMAND zeros(1,4)];
index=2;
[command_string, index] = code_int16(command_string, index, int16(ich));
[command_string, index] = code_int16(command_string, index, int16(qch));

encoding_and_send(board, CRC_ENGINE, command_string);

pause(.01);
stream_in = read_answer_crc(board, CRC_ENGINE);
if (isempty(stream_in)==1)
    ret_code=1;
    return;
end

if (stream_in(1)==COMMAND)
    ret_code = 0;
else
    ret_code = 1;
end;


end

