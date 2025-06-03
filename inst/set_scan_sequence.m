% *************************************************
% ARIA Sensing srl 2024
% Confidential-reserved
% *************************************************

%set the scan sequence, rxmask and txmask must have the same size.
%input is a bitwise sequece where every antenna is selected
function [ ret_code ] = set_scan_sequence(board, rxmask, txmask)

global CRC_ENGINE;

%check structure
if (length(rxmask) ~= length(txmask))
  ret_code = -1;
end
if (any(rxmask>15) || any(txmask>15) || any(rxmask < 0) || any(txmask<0))
  ret_code = -2;
end

%encode according to message's format
numSeq = uint8(length(rxmask));
oarray = uint8(txmask * 16 + rxmask);

COMMAND    = uint8(8);
command_string = [COMMAND zeros(1,numSeq)];
index=2;

##[command_string, index] = code_int8(command_string, index, numSeq);
[command_string, index] = code_uint8(command_string, index, oarray);

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

