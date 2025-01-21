% *************************************************
% Cover Sistemi srl 2018
% Confidential-reserved
% *************************************************
function [ index, value ] = get_uint8( data_stream, index_in )
% int8 Decode the current stream looking for 1byte to form a 
% single 8bit value. If the search goes beyond input
% array, index is assigned -1 value
imax = length(data_stream);

index = index_in +1;
if (index > imax+1)
    value=0;
    index=-1;
    return;
end
array_dest =  data_stream(index_in);
value = typecast(uint8(array_dest),'uint8');

end

