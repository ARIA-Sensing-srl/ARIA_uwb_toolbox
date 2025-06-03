% *************************************************
% Cover Sistemi srl 2018
% Confidential-reserved
% *************************************************

%index
%(SNR = 0, SNR1 = 1, SNR2 = 2)

function [ ret_code, valOut ] = set_SNR(board, val, snrindex)
%---------------------------------------------------------------------
% Set the offset in meter. The offset is the distance of the first sample
% in radar acquisition
% global DEFINE_OCTAVE;
global CRC_ENGINE;
valOut = -1;

% START_CHAR = uint8(hex2dec('FF'));

switch(snrindex)
    case 0
        COMMAND = uint8(110);
    case 1
        COMMAND = uint8(44);
    case 2
        COMMAND = uint8(45);
    case 3
        COMMAND = uint8(95);
    otherwise
        ret_code = 1;
        return;
end

% END_CHAR   = uint8(hex2dec('00'));

command_string = [  COMMAND zeros(1,4)];

index=2;
if (isempty(val)==0)
  [command_string, index] = code_float(command_string, index, val);
else
  val = int32(-1);
  [command_string, index] = code_int32(command_string, index, val);
endif

% command_string(index) = END_CHAR;

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

ret_code=0;
if (stream_in(1)==COMMAND)
    [~,valOut] = get_float(stream_in,2);
else
    ret_code = 1;
    return;
end;



end

