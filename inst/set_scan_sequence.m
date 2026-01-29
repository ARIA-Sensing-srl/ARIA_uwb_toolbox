% *************************************************
% ARIA Sensing srl 2024
% Confidential-reserved
% *************************************************

%set the scan sequence, rxmask and txmask must have the same size.
%input is a bitwise sequece where every antenna is selected
function [ ret_code, rxOut, txOut ] = set_scan_sequence(board, rxmask, txmask)

rxOut= [];
txOut = [];
ret_code = 0;

global CRC_ENGINE;

COMMAND    = uint8(8);
if (isempty(rxmask) || isempty(txmask))
  command_string = [COMMAND];
else
  %check structure
  if (length(rxmask) ~= length(txmask))
    ret_code = -1;
  end
  if (any(rxmask>15) || any(txmask>15) || any(rxmask < 0) || any(txmask<0))
    ret_code = -2;
  end

  %encode according to message's format
  numSeq = uint8(length(rxmask));
  oarray = uint8(txmask * 16 + rxmask);


  command_string = [COMMAND zeros(1,numSeq)];
  index=2;
  [command_string, index] = code_uint8(command_string, index, oarray);
end

encoding_and_send(board, CRC_ENGINE, command_string);
pause(.01);
stream_in = read_answer_crc(board, CRC_ENGINE);
if (isempty(stream_in)==1)
    ret_code=1;
    return;
end

if (stream_in(1)==COMMAND)

    if (length(stream_in)>1)
      scanencoded=stream_in(2:end);
      numScans = length(scanencoded);

      #decode
      rxOut = bitand(uint8(scanencoded), 0xF);
      txOut = bitand(bitshift(uint8(scanencoded),-4), 0xF);
    end

    ret_code = 0;
else
    ret_code = 1;
end;


end

