% *************************************************
% Cover Sistemi srl 2018
% Confidential-reserved
% *************************************************
function [ ret_code, v ] = set_slow_time_gain(board, gain)
%------------------------------------------------
%------------------------------------------------
% Set the gain across slow-time axis
% global DEFINE_OCTAVE;
global CRC_ENGINE;
% START_CHAR = uint8(hex2dec('FF'));
COMMAND    = uint8('i');
% END_CHAR   = uint8(hex2dec('00'));

ret_code = 0;
v = [];
command_string = [  COMMAND zeros(1,16)];

index=2;
if (isempty(gain))
  ui16fps = uint16(0xFFFF);
else
  ui16fps = uint16(gain);
end
[command_string, index] = code_int16(command_string, index, ui16fps);
% command_string(index) = END_CHAR;
command_string = command_string(1:index-1);
encoding_and_send(board, CRC_ENGINE, command_string);
% if (DEFINE_OCTAVE==0)
% 	flushinput(board);
% 	fwrite(board,command_string);
% else
% 	srl_flush(board);
% 	srl_write(board,command_string);
% end;
% Read answer
pause(.01);
stream_in = read_answer_crc(board, CRC_ENGINE);
if (isempty(stream_in)==1)
    ret_code=1;
    return;
end

if (stream_in(1)=='i')
    [~,v] = get_uint16(stream_in,2);

else
    ret_code = 1;
    return;
end;

end

