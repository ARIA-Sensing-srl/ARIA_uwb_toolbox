
function [ ret_code, adcfreq] = get_adcfreq(board)
  COMMAND    = uint8(0x05);
  adcfreq = 0;

  reqpayload = uint16(0xFFFF); #don't care

  command_string = [  COMMAND zeros(1,16)];

  index=2;
  [command_string, index] = code_int16(command_string, index, reqpayload);


  global CRC_ENGINE;
  command_string = command_string(1:index-1);
  encoding_and_send(board, CRC_ENGINE, command_string);

  pause(.01);
  stream_in = read_answer_crc(board, CRC_ENGINE);
  if (isempty(stream_in)==1)
      ret_code=1;
      return;
  end

  if (stream_in(1)==COMMAND)
      [~,adcfreq] = get_int16(stream_in,2);
      ret_code = 0;
  else
      ret_code = 1;
  end;
end

