% *************************************************
% Cover Sistemi srl 2018
% Confidential-reserved
% *************************************************
function [ ret_code ] = set_multistreammode(board, ms)
% Start the radar acquisition
% global DEFINE_OCTAVE;
% START_CHAR = uint8(hex2dec('FF'));
COMMAND    = 0x02;
% END_CHAR   = uint8(hex2dec('00'));
u16ms = uint16(ms);

command_string = [  COMMAND zeros(1,16)];

index=2;
[command_string, index] = code_int16(command_string, index, u16ms);
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
    [~,v] = get_int16(stream_in,2);
    if (v~=ms)
        ret_code = 1;
    else
        ret_code = 0;
    end;
else
    ret_code = 1;
end;


end

