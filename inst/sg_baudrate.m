function [ ret_code,retBaud ] = sg_baudrate(board, baud)
%---------------------------------------------------------------------
% Set the retBaud in meter. The retBaud is the distance of the first sample
% in radar acquisition
% global DEFINE_OCTAVE;
retBaud = -1;
ret_code = 0;

% START_CHAR = uint8(hex2dec('FF'));
COMMAND    = uint8(0x04);
% END_CHAR   = uint8(hex2dec('00'));

command_string = [  COMMAND zeros(1,16)];

index=2;
[command_string, index] = code_int32(command_string, index, baud);
% command_string(index) = END_CHAR;

global CRC_ENGINE;
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
if (stream_in(1)==COMMAND)
    [~,retBaud] = get_float(stream_in,2);
else
    ret_code = 1;
    return;
end;



end

