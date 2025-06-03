% *************************************************
% Cover Sistemi srl 2018
% Confidential-reserved
% *************************************************
function [ output_data,elapse_time ] = read_raw_data( board, downconversion )
%Read_raw_data:  ask for a radar frame
global CRC_ENGINE;

elapse_time = [];
output_data=[];

COMMAND    = uint8('d');
command_string =  COMMAND ;
encoding_and_send(board, CRC_ENGINE, command_string);

pause(.01);
rxBuf = read_answer_crc(board, CRC_ENGINE);
if (isempty(rxBuf)==1)
    return;
end

if (length(rxBuf) < 5)
    return; %no header
end

%decode header
rxI=1;
[rxI, echoCmd] = get_int8(rxBuf, rxI);
[rxI, number_of_samples] = get_int16(rxBuf, rxI);
[rxI, ~]=get_int16(rxBuf, rxI);

if (echoCmd ~= 'd')
    return;
end


output_data = zeros(1,number_of_samples);
temp_data = output_data;

stream_in=rxBuf(rxI: end);

% Elapsed time
dest_index = 1;
index = 1;
[index,elapse_time] = get_int16(stream_in, index);

real_rescale = 1.0;
real_offset  = 0.0;
imag_rescale = 1.0;
imag_offset  = 0.0;
% In case of DDC, the first data are offset and scale values
if (downconversion==1)||(downconversion==2)
    [index, real_offset] = get_float(stream_in,index);
    [index, real_rescale] = get_float(stream_in,index);
    % Starting from v1.5 LT103OEM, we may have also DDC Complex Envelop
    if (downconversion==2)
        [index, imag_offset] = get_float(stream_in,index);
        [index, imag_rescale] = get_float(stream_in,index);
    end
end
    

for i=1:number_of_samples
    if (downconversion==0)||(downconversion==3)
        [index, df] = get_float(stream_in,index);
    end
    
    if (downconversion==1)||(downconversion==2)
        [index,df] = get_uint8(stream_in,index);
    end
    
    if (index == -1)
        output_data = [];
        return;
    end
    
    % Stack data into destination array
    temp_data(dest_index) = single(df);
    dest_index = dest_index + 1;
end
%------------------------------------------------
% Raw data
if downconversion==0
    output_data = temp_data;
    return;
end
%------------------------------------------------
% Module only DDC
if downconversion==1
    output_data = (temp_data / real_rescale) + real_offset;
%     output_data = exp(output_data);
    return;
end

%------------------------------------------------
% Complex DDC
if downconversion==2
    overall_length = number_of_samples/2;
    nStartIm = overall_length+1;
    temp_complex = zeros(1,overall_length);
    
    for nStartRe = 1:overall_length
        temp_complex(nStartRe)=complex(temp_data(nStartRe),temp_data(nStartIm));
        nStartIm = nStartIm+1;
    end
    output_data = complex( real(temp_complex)/real_rescale + real_offset,...
                           imag(temp_complex)/imag_rescale + imag_offset );
    return;
end
%-----------------------------------------------------------
if downconversion==3
    overall_length = number_of_samples/2;
    nStartIm = overall_length+1;
    output_data = zeros(1,overall_length);
    
    for nStartRe = 1:overall_length
        output_data(nStartRe)=complex(temp_data(nStartRe),temp_data(nStartIm));
        nStartIm = nStartIm+1;
    end
    return;
end

end

