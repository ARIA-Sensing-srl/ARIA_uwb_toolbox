% *************************************************
% Cover Sistemi srl 2018
% Confidential-reserved
% *************************************************
function [ index, value ] = get_q16( data_stream, index_in )
% get_float Decode the current stream looking for 2bytes to form a 
% single q16 value. If the search goes beyond input
% array, index is assigned -1 value
array_dest = [0 0];
index_dest = 1;
imax = length(data_stream);


global CRC_ENGINE;
if (isempty(CRC_ENGINE)==0)
    index = index_in +2;
    if (index > imax+1)
        value=0;
        index=-1;
        return;
    end
    array_dest =  data_stream(index_in:index_in+1);
    value = typecast(uint8(array_dest),'int16');
else
    while (index_dest <= 2)

        if index_in > imax
            index = -1;
            return;
        end

        curr_byte = uint8(data_stream(index_in));
        if (curr_byte == hex2dec('80'))
            % Get next number
            index_in = index_in + 1;
            if index_in > imax
                index = -1;
                return;
            end

            curr_byte = uint8(data_stream(index_in));
            if (curr_byte == hex2dec('FE')) % Alternate start
                curr_byte = hex2dec('FF'); % Start
            elseif (curr_byte == hex2dec('01')) % Alternate stop
                curr_byte = hex2dec('00'); % Stop
            elseif (curr_byte == hex2dec('81')) % Alternate check
                curr_byte = hex2dec('80'); % Check
            end
        end

        array_dest(index_dest) = curr_byte;
        index_dest = index_dest+1;
        index_in   = index_in+1;
    end
        value = typecast(uint8(array_dest),'int16');
        index = index_in;

    return;
end

