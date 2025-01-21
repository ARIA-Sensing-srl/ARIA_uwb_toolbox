% *************************************************
% Cover Sistemi srl 2018
% Confidential-reserved
% *************************************************
function [ data_stream, index ] = code_int32( stream_in, index_in, value)
% code_float Encode a float (single precision) into a set of 4 or more bytes
% according to coding scheme
    data_stream = stream_in;
    imax = length(stream_in);
    data32 = typecast(int32(value),'uint8');

    numBytes = length(data32);
    index = index_in + numBytes;
    if (index > imax+1)
        index = -1;
        return;
    end
    for N=1:numBytes
        data_stream(index_in-1+N)=data32(N);
    end    
end
