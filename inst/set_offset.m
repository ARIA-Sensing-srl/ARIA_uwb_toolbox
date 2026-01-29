% *************************************************
% Cover Sistemi srl 2018
% Confidential-reserved
% *************************************************
function [ ret_code,offset ] = set_offset(board, offset_in)
%---------------------------------------------------------------------
% Set the offset in meter. The offset is the distance of the first sample
% in radar acquisition
% global DEFINE_OCTAVE;
offset = -1;
ret_code = 0;

% START_CHAR = uint8(hex2dec('FF'));
COMMAND    = uint8('0');
% END_CHAR   = uint8(hex2dec('00'));

command_string = [  COMMAND zeros(1,16)];

index=2;
if (isempty(offset_in))
	[command_string, index] = code_int32(command_string, index, int32(-1));
else
	[command_string, index] = code_float(command_string, index, offset_in);
end
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
if (stream_in(1)=='0')
    [~,offset] = get_float(stream_in,2);
else
    ret_code = 1;
    return;
end;



end

