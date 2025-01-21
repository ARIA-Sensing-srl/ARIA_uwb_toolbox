

function [ output_data_final,elapse_time, fmt ] = msgdec_read_volume( rxBuf )
rxindex=1;
[rxindex,columns] = get_int16(rxBuf,rxindex);
[rxindex,etime_fmt] = get_int16(rxBuf,rxindex);

[rxindex,numEl] = get_int8(rxBuf,rxindex);
[rxindex,numTheta] = get_int8(rxBuf,rxindex);

stream_data = rxBuf(rxindex:end);

totalsamples = uint32(numEl)*uint32(numTheta)*uint32(columns);

elapse_time = double(bitand(uint16(etime_fmt), uint16(0xFFF)));
fmt = bitshift(etime_fmt,-12);

[binsize_bytes, typecast_str] = get_fmt_size_bytes(fmt);

if (binsize_bytes == 0)
  return; %unsupported format
end

%check data size
stream_data_size = length(stream_data);
expected_data_size = int32(totalsamples) * int32(binsize_bytes);

if (stream_data_size ~= expected_data_size)
  return;
end


if (strcmp(typecast_str, "halfprecision") == 0)
  output_data = typecast(uint8(stream_data), typecast_str);
else
  output_data_u16 = typecast(uint8(stream_data), "uint16");
  output_data = f16tosingle(output_data_u16);
end

output_data_final = transpose(output_data);
output_data_final = (reshape(output_data_final, columns, numEl, numTheta));



endfunction

