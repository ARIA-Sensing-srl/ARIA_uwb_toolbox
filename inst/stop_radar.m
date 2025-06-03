% *************************************************
% Cover Sistemi srl 2018
% Confidential-reserved
% *************************************************
function [ ret_code ] = stop_radar(board)
% Stop the radar acquisition
global DEFINE_OCTAVE;
global CRC_ENGINE;
% START_CHAR = uint8(hex2dec('FF'));
COMMAND    = uint8('o');
% END_CHAR   = uint8(hex2dec('00'));
command_string = [COMMAND];

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
if (stream_in(1)=='o')
    ret_code = 0;
else
    ret_code = 1;
end;


end

