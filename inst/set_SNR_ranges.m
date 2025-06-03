% *************************************************
% Cover Sistemi srl 2018
% Confidential-reserved
% *************************************************

% range_index from 0 to 2

function [ ret_code, valOut ] = set_SNR_ranges(board, val, range_index)
%---------------------------------------------------------------------
% Set the offset in meter. The offset is the distance of the first sample
% in radar acquisition
% global DEFINE_OCTAVE;
global CRC_ENGINE;
valOut = [];
ret_code = 0;

% START_CHAR = uint8(hex2dec('FF'));
if (isempty(range_index) == 0)
  %legacy comman, suitable for NOT HYDROGEN version
  switch(range_index)
      case 0
          COMMAND = uint8('1');
      case 1
          COMMAND = uint8('2');
      case 2
          COMMAND = uint8('3');
      otherwise
          ret_code = 1;
          return;
  end

  % END_CHAR   = uint8(hex2dec('00'));

  command_string = [  COMMAND zeros(1,4)];

  index=2;
  [command_string, index] = code_float(command_string, index, val);
  % command_string(index) = END_CHAR;

  encoding_and_send(board, CRC_ENGINE, command_string);
  % if (DEFINE_OCTAVE==0)
  % 	flushinput(board);
  % 	fwrite(board,command_string);
  % else
  % 	srl_flush(board);
  % 	srl_write(board,command_string);
  % end;
  % Read answer
  pause(.01);
  stream_in = read_answer_crc(board, CRC_ENGINE);
  if (isempty(stream_in)==1)
      ret_code=1;
      return;
  end

  ret_code=0;
  if (stream_in(1)==COMMAND)
      [~,valOut] = get_float(stream_in,2);
  else
      ret_code = 1;
      return;
  end;
else

  numItems = 3;
  if (isempty(val) == 0)
    if (length(val) ~= numItems)
      ret_code = 1; #not valid
      return;
    endif
  endif

  COMMAND = uint8('1');

  command_string = [  COMMAND zeros(1,12)]; #3 float
  index=2;
if (isempty(val) == 0)
  for N = 1:numItems
    [command_string, index] = code_float(command_string, index, val(N));
  endfor
else
  val = int32(-1);
  [command_string, index] = code_int32(command_string, index, val);
endif


  encoding_and_send(board, CRC_ENGINE, command_string);
  pause(.01);
  #read response
  stream_in = read_answer_crc(board, CRC_ENGINE);

  #check if transaction occurred
  if (isempty(stream_in)==1)
      ret_code=1;
      return;
  end

  if (stream_in(1)==COMMAND)
      index = 2;
      for N = 1:numItems
        [index,valOut(N)] = get_float(stream_in,index);
      endfor
  else
      ret_code = 1;
      return;
  end;
end

endfunction

