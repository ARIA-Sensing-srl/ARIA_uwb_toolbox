

function [ output_data_final,elapse_time, fmt ] = msgdec_raw_data_multiple( rxBuf )


output_data_final=[];
elapse_time=[];
fmt = [];
rxindex = 1;
[rxindex,sampleforscan] = get_int16(rxBuf,rxindex);
[rxindex,etime_fmt] = get_int16(rxBuf,rxindex);
[rxindex,totalsamples] = get_int16(rxBuf,rxindex);
stream_data = rxBuf(rxindex:end);


elapse_time = double(bitand(uint16(etime_fmt), uint16(0xFFF)))/1000; #elapsed time in ms
fmt = bitshift(etime_fmt,-12);

[binsize_bytes, typecast_str] = get_fmt_size_bytes(fmt);

if (binsize_bytes == 0)
  return; %unsupported format
end

%check data size
stream_data_size = length(stream_data);
expected_data_size = int32(totalsamples) * int32(binsize_bytes) * 2;

if (stream_data_size ~= expected_data_size)
  return;
end
numChannels = totalsamples/sampleforscan;

#output_data = typecast(uint8(stream_data), typecast_str);
if (strcmp(typecast_str, "halfprecision") == 0)
  output_data = typecast(uint8(stream_data), typecast_str);
else
  output_data_u16 = typecast(uint8(stream_data), "uint16");
  output_data = f16tosingle(output_data_u16);
end
output_data_final = complex(output_data(1:2:end), output_data(2:2:end));

%process only raw vector
if (size(output_data_final, 1) ~= 1)
  output_data_final = transpose(output_data_final);
end

output_data_final = transpose(reshape(output_data_final, sampleforscan, numChannels));

