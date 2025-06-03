function [ ret_code ] = set_data_channel(board, dataChannel)
%-------------------------------------------------
% Elaboration type
%-------------------------------------------------
% 0-3: channel 
% 4: combined
% 5: all channels (need special readout routine)
% 0xFF: No modifications = Interrogate on current Elaboration value

% Start the radar acquisition
% global DEFINE_OCTAVE;
% START_CHAR = uint8(hex2dec('FF'));
COMMAND    = uint8('$');
% END_CHAR   = uint8(hex2dec('00'));
ui8elab = uint8(dataChannel);

command_string = [  COMMAND zeros(1,16)];

index=2;
[command_string, index] = code_int8(command_string, index, ui8elab);
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

if (stream_in(1)=='$')
    [~,v] = get_int8(stream_in,2);
    if (v~=ui8elab)
        ret_code = 1;
    else
        ret_code = 0;
    end;
else    
    ret_code = 1;
end;


end

