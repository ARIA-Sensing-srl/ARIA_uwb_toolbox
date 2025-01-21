% *************************************************
% Cover Sistemi srl 2018
% Confidential-reserved
% *************************************************
function [ index, value ] = get_uint16( data_stream, index_in )
% get_float Decode the current stream looking for 2bytes to form a 
% single int16 value. 
imax = length(data_stream);

index = index_in +2;
if (index > imax+1)
    value=0;
    index=-1;
    return;
end
array_dest  =  data_stream(index_in:index_in+1);
value       =  typecast(uint8(array_dest),'uint16');

end

