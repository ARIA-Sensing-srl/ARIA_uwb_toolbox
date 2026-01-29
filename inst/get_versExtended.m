% *************************************************
% Aria sensing srl 2026
% *************************************************
#0: Application processor
#1: Application processor lib build
#2: Coprocessor
#3: Coprocessor library build
function [ rc , rval] = get_versExtended(board, val)

rc = 0;
rval = [];


COMMAND    = uint8(0x56);
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
    if (length(stream_in)>1)
      rval = char(stream_in(2:end));
    endif
else
    rc = 1;
end

end

