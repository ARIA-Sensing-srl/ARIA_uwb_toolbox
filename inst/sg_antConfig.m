% *************************************************
% ARIA Sensing srl 2024
% Confidential-reserved
% *************************************************

%rxList (structure ID, X , Y, delay, ampl) units are meters and seconds
function [ ret_code, retRxList, retTxList ] = sg_antConfig(board, rxList, txList)

global CRC_ENGINE;

%encode according to message's format
retRxList = [];
retTxList = [];
IDTxEncodeOffset = 4;
COMMAND    = uint8('v');
command_string = [COMMAND zeros(1,128)];
index=2;

if (isempty(rxList) == 0)
  for N = 1:length(rxList)
    [command_string, index] = code_int8(command_string, index, uint8(rxList.ID));
    [command_string, index] = code_float(command_string, index, single(rxList.X));
    [command_string, index] = code_float(command_string, index, single(rxList.Y));
    [command_string, index] = code_float(command_string, index, single(rxList.delay));
    [command_string, index] = code_float(command_string, index, single(rxList.ampl));
  endfor
endif

if (isempty(txList) == 0)
  for N = 1:length(txList)
    [command_string, index] = code_int8(command_string, index, uint8(IDTxEncodeOffset+txList.ID));
    [command_string, index] = code_float(command_string, index, single(txList.X));
    [command_string, index] = code_float(command_string, index, single(txList.Y));
    [command_string, index] = code_float(command_string, index, single(txList.delay));
    [command_string, index] = code_float(command_string, index, single(txList.ampl));
  endfor
endif


command_string = command_string(1:(index-1));

encoding_and_send(board, CRC_ENGINE, command_string);
pause(.01);
stream_in = read_answer_crc(board, CRC_ENGINE);

if (isempty(stream_in)==1)
    ret_code=1;
    return;
end

if (stream_in(1)==COMMAND)
    [retRxList, retTxList]= msgdec_ant_config (stream_in(2:end));
    if (isempty(retRxList))
      ret_code = 1;
      return;
    endif
    ret_code = 0;
else
    ret_code = 1;
end;
end

