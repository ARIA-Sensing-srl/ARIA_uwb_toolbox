% *************************************************
% Cover Sistemi srl 2018
% Confidential-reserved
% *************************************************
function [ data_stream, index ] = code_float( stream_in, index_in, value)
% code_float Encode a float (single precision) into a set of 4 or more bytes
% according to coding scheme
    data_stream = stream_in;
    imax = length(stream_in);

    data8 = typecast(single(value),'uint8');

    numBytes = length(data8);
    index = index_in + numBytes;
    if (index > imax+1)
        index = -1;
        return;
    end
    for N=1:numBytes
        data_stream(index_in-1+N)=data8(N);
    end;    
end

