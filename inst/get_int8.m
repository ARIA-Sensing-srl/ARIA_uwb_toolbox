% *************************************************
% Cover Sistemi srl 2018
% Confidential-reserved
% *************************************************
function [ index, value ] = get_int8( data_stream, index_in, varargin )
% int8 Decode the current stream looking for 1byte to form a
% single 8bit value. If the search goes beyond input
% array, index is assigned -1 value

imax = length(data_stream);
if (length(varargin) > 0)
  arraysize = varargin{1};
else
  arraysize = 1;
endif

index = index_in + arraysize;
if (index > imax+1)
    value=0;
    index=-1;
    return;
end
##array_dest =  data_stream(index_in);
array_dest =  data_stream(index_in:(index-1));
value = typecast(uint8(array_dest),'int8');

end

