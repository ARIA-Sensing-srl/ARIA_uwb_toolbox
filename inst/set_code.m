% *************************************************
% ARIA Sensing srl 2024
% Confidential-reserved
% *************************************************

%set code sequece
function [ ret_code ] = set_code(board, sequence)

global CRC_ENGINE;

%check structure
if (any(abs(sequence) > 1))
  if (any(sequence>2))
    ret_code = -2;
  endif
end

%encode according to message's format
oarray = int8(sequence);
numSeq = uint8(length(oarray));


COMMAND    = uint8(13);
command_string = [COMMAND zeros(1,numSeq)];
index=2;

[command_string, index] = code_int8(command_string, index, oarray);

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

