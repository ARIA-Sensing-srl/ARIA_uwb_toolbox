% *************************************************
% Cover Sistemi srl 2018
% Confidential-reserved
% *************************************************
function [ ret_code, customer_code, hw_code, fw_id_ver,feature_set,fw_id_subver ] = get_version_fw(board)
% Start the radar acquisition
% global DEFINE_OCTAVE;

COMMAND    = uint8(hex2dec('01'));

command_string = [  COMMAND zeros(1,16)];

index=2;

global CRC_ENGINE;
command_string = command_string(1:index-1);
encoding_and_send(board, CRC_ENGINE, command_string);

pause(.01);
stream_in = read_answer_crc(board, CRC_ENGINE);
if (isempty(stream_in)==1)
    ret_code=1;
    return;
end

if (stream_in(1)==COMMAND)
    ret_code = 0;
    % According to versioning format,
    % 1) The first WORD is the Customer Code
    [index, customer_code] = get_uint16(stream_in,index);
    % 2) The second WORD is hardware code
    [index, hw_code]        = get_uint16(stream_in,index);
    % 2) FW Version, 1 Byte
    [index, fw_id_ver] = get_uint8(stream_in,index);
    % 2) Hardware code
    [index, feature_set] = get_uint8(stream_in,index);
    % 2) Hardware code
    [~, fw_id_subver] = get_uint16(stream_in,index);
 
else    
    ret_code = 1;
    return;
end


end

