% *************************************************
% Cover Sistemi srl 2018
% Confidential-reserved
% *************************************************
function [ index, value ] = get_int32( data_stream, index_in )
% get_float Decode the current stream looking for 4bytes to form a 
% single int32 value. 

imax = length(data_stream);

index = index_in +4;
if (index > imax+1)
    value=0;
    index=-1;
    return;
end
array_dest =  data_stream(index_in:index_in+3);
value = typecast(uint8(array_dest),'int32');

end
