% *************************************************
% ARIA Sensing srl 2024
% Confidential-reserved
% *************************************************

%Set the data format returned when a data frame is required
%0: Q.7
%1: Q.15
%2: Q.31
%3: F32
%4: F16

function [ ret_code, v] = set_data_fmt(board, fmt)
%-------------------------------------------------
% Tx Power
%-------------------------------------------------
% 0-7
% 0xFF query
v = [];
ret_code = 0;
global CRC_ENGINE;
COMMAND    = uint8(10);

if (isempty(fmt))
	fmt = uint8(0xFF);
else
	fmt = uint8(fmt);
end

command_string = [COMMAND zeros(1,1)];
index=2;
[command_string, index] = code_int8(command_string, index, fmt);

encoding_and_send(board, CRC_ENGINE, command_string);

pause(.01);
stream_in = read_answer_crc(board, CRC_ENGINE);
if (isempty(stream_in)==1)
    ret_code=1;
    return;
end

if (stream_in(1)==COMMAND)
    [~,v] = get_int8(stream_in,2);
else
    ret_code = 1;
end;


end

