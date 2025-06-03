% *************************************************
% Aria Sensing srl 2024
% Confidential-reserved
% *************************************************


function [ ret_code, curCfg ] = sg_led(board, override, red ,green ,blue)
global CRC_ENGINE;
ret_code = 0;
curCfg = [];

COMMAND = uint8(0x06);
command_string = [  COMMAND zeros(1,4)];

isquery = isempty(override) + isempty(red) + isempty(green) + isempty(blue);

index=2;
if (isquery == 0)
  [command_string, index] = code_int8(command_string, index, override);
  [command_string, index] = code_int8(command_string, index, red);
  [command_string, index] = code_int8(command_string, index, green);
  [command_string, index] = code_int8(command_string, index, blue);
else
  [command_string, index] = code_int8(command_string, index, 0xFF);
endif


encoding_and_send(board, CRC_ENGINE, command_string);

pause(.01);
stream_in = read_answer_crc(board, CRC_ENGINE);
if (isempty(stream_in)==1)
    ret_code=1;
    return;
end

ret_code=0;
if (stream_in(1)==COMMAND)
    index = 2;
    [index, curCfg.override] = get_int8(stream_in, index);
    [index, curCfg.red] = get_int8(stream_in, index);
    [index, curCfg.green] = get_int8(stream_in, index);
    [index, curCfg.blue] = get_int8(stream_in, index);
else
    ret_code = 1;
    return;
end;

end

