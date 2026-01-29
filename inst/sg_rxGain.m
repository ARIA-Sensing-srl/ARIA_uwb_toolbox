% *************************************************
% Aria sensing srl 2026
% *************************************************
function [ rc , rval] = sg_rxGain(board, val)

rc = 0;
rval = [];

if (isempty(val))
  val = 0xFF;
endif

COMMAND    = uint8(0x82);
command_string = [COMMAND zeros(1,1)];
[command_string, index] = code_int8(command_string, 2, uint8(val));

encoding_and_send(board, [], command_string);
pause(.01);
stream_in = read_answer_crc(board, []);
if (isempty(stream_in)==1)
    rc=1;
    return;
end

if (stream_in(1)==COMMAND)
    [~,rval] = get_int8(stream_in,2);
    if (isempty(rval))
      rc = 1;
    endif
else
    rc = 1;
end

end

