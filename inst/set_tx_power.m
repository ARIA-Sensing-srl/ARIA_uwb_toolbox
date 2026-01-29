% *************************************************
% Cover Sistemi srl 2018
% Confidential-reserved
% *************************************************
function [ ret_code, v ] = set_tx_power(board, tx_power)
%-------------------------------------------------
% Tx Power
%-------------------------------------------------
% 0-7
% 0xFF query
ret_code = 0 ;
v = [];

global CRC_ENGINE;
COMMAND    = uint8('p');

if (isempty(tx_power))
	ui8tx_power = uint8(0xFF);
else
	ui8tx_power = uint8(tx_power);
end

command_string = [COMMAND zeros(1,1)];
index=2;
[command_string, index] = code_int8(command_string, index, ui8tx_power);

encoding_and_send(board, CRC_ENGINE, command_string);

pause(.01);
stream_in = read_answer_crc(board, CRC_ENGINE);
if (isempty(stream_in)==1)
    ret_code=1;
    return;
end

if (stream_in(1)=='p')
    [~,v] = get_int8(stream_in,2);
else
    ret_code = 1;
end;


end

