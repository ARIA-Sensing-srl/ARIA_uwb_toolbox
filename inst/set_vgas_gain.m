% *************************************************
% ARIA Sensing srl 2024
% Confidential-reserved
% *************************************************

%set VGA gains

function [ ret_code, ichOut, qchOut ] = set_vgas_gain(board, ich, qch)

global CRC_ENGINE;
COMMAND    = uint8(14);

ret_code = 0;
ichOut = [];
qchOut = [];

if (isempty(ich) || isempty(qch))
	command_string = [COMMAND];

else
  command_string = [COMMAND zeros(1,4)];
  index=2;
  [command_string, index] = code_int16(command_string, index, int16(ich));
  [command_string, index] = code_int16(command_string, index, int16(qch));
end

encoding_and_send(board, CRC_ENGINE, command_string);

pause(.01);
stream_in = read_answer_crc(board, CRC_ENGINE);
if (isempty(stream_in)==1)
    ret_code=1;
    return;
end

if (stream_in(1)==COMMAND)
    [ind,ichOut] = get_int16(stream_in,2);
    [~,qchOut] = get_int16(stream_in,ind);

    ret_code = 0;
else
    ret_code = 1;
end;


end

