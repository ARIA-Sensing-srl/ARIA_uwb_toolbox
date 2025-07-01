function [ ret_code, retPwrmode ] = set_pwrSaveMode(board, pwrMode)
retPwrmode =[];
ret_code=1;
COMMAND    = uint8(0x06);

if (isempty(pwrMode)==0)
  modeEncoded = uint8(pwrMode);
else
  modeEncoded = uint8(0xFF);
endif


command_string = [  COMMAND zeros(1,16)];

index=2;
[command_string, index] = code_int8(command_string, index, modeEncoded);
% command_string(index) = END_CHAR;
global CRC_ENGINE;
command_string = command_string(1:index-1);
encoding_and_send(board, CRC_ENGINE, command_string);

pause(.01);
stream_in = read_answer_crc(board, CRC_ENGINE);
if (isempty(stream_in)==1)
    ret_code=1;
    return;
end

if (stream_in(1)==COMMAND)
    [~,retPwrmode] = get_int8(stream_in,2);
    ret_code = 0;
else
    ret_code = 1;
end;


end

