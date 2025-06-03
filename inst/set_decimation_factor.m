% *************************************************
% Cover Sistemi srl 2018
% Confidential-reserved
% *************************************************
function [ ret_code ] = set_decimation_factor(board, dc_factor)
%------------------------------------------------
% Set the decimation factor
COMMAND    = uint8('q');
command_string = [  COMMAND zeros(1,16)];

index=2;
ui8_ddc_on = uint8(dc_factor);
[command_string, index] = code_int8(command_string, index, ui8_ddc_on);
% command_string(index) = END_CHAR;
global CRC_ENGINE;
command_string = command_string(1:index-1);
encoding_and_send(board, CRC_ENGINE, command_string);

% Read answer
pause(.01);
stream_in = read_answer_crc(board, CRC_ENGINE);
if (isempty(stream_in)==1)
    ret_code=1;
    return;
end

if (stream_in(1)=='q')
    [~,v] = get_int8(stream_in,2);
    if (v~=ui8_ddc_on)
        ret_code = 1;
        return;
    else
        ret_code = 0;
    end;
else    
    ret_code = 1;
    return;
end;

end

