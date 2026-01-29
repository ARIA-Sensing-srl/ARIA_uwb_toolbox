% *************************************************
% Aria sensing srl 2026
% *************************************************
function [ rc , rval] = sg_staticObjMapUpdateTime(board, val)

rc = 0;
rval = [];

COMMAND    = uint8(0x81);
command_string = [COMMAND zeros(1,4)];

if (isempty(val))
  [command_string, index] = code_int32(command_string, 2, uint32(-1));
else
  [command_string, index] = code_float(command_string, 2, single(val));
endif


encoding_and_send(board, [], command_string);
pause(.01);
stream_in = read_answer_crc(board, []);
if (isempty(stream_in)==1)
    rc=1;
    return;
end

if (stream_in(1)==COMMAND)
    [~,rval] = get_float(stream_in,2);
    if (isempty(rval))
      rc = 1;
    endif
else
    rc = 1;
end

end

