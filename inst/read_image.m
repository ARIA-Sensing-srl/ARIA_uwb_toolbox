% *************************************************
% Cover Sistemi srl 2018
% Confidential-reserved
% *************************************************
function [ output_data_final,elapse_time, fmt ] = read_image( board )
%Read_raw_data:  ask for a radar frame
%read raw data 2d, set_channel must be 5 (all)
global CRC_ENGINE;

elapse_time = [];
output_data_final=[];
fmt = [];

COMMAND    = uint8(15);
command_string =  COMMAND ;
encoding_and_send(board, CRC_ENGINE, command_string);
resRx=[];


[rxBuf, resRx] = read_answer_crc(board, CRC_ENGINE, resRx);
if (isempty(rxBuf)==1)
    return;
end

if (length(rxBuf) < 8)
    return; %no header or data header +cmd ==7, at least one more for data
end

%decode header
[rxindex,retcmd] = get_int8(rxBuf,1);
if (retcmd ~= uint8(15))
  return;
end

[ output_data_final,elapse_time, fmt ] = msgdec_image(rxBuf(2:end));



end

