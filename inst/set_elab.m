function [ ret_code, v ] = set_elab(board, elaboration_type)

ret_code = 0;
v = [];
%-------------------------------------------------
% Elaboration type
%-------------------------------------------------
% 0: Get Raw Radar data
% 1: Apply adaptive cluttermap and get MTI data
% 2: Evaluate analytic signal magnitude of MTI data
% 3: Evaluate SNR and extract MTI target data
% 4: Get target data
% 0x05 <-> 0xFE DON'T USE!
% 0xFF: No modifications = Interrogate on current Elaboration value

% Start the radar acquisition
% global DEFINE_OCTAVE;
% START_CHAR = uint8(hex2dec('FF'));
COMMAND    = uint8('e');
% END_CHAR   = uint8(hex2dec('00'));

if (isempty(elaboration_type))
  ui8elab = uint8(0xFF);
else
  ui8elab = uint8(elaboration_type);
end

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

if (stream_in(1)=='e')
    [~,v] = get_int8(stream_in,2);

else
    ret_code = 1;
end;


end

