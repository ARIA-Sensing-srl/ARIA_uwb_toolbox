% *************************************************
% Aria Sensing srl 2024
% Confidential-reserved
% *************************************************

#set calman filter params

#prune blob behind a closer blob
#params.phiSpan         phase difference (in radiants)
#params.amplDecay       amplitude decay (in m)

#separate blobs along phi
#params.topDetWndSize   wnd size t detect top
#params.top2SideThr     Thr to limit amplitude of a single blob
#params.sideGuard        Number of samples on guard range

#if param == [] request data

function [ ret_code, paramsOut ] = set_blobfilter(board, params)

paramsOut=[];
ret_code = 0;

COMMAND    = uint8('2');
payloadLgth = 14;

command_string = [COMMAND];
index=2;

if (isempty(params) == 1)
   command_string = [command_string zeros(1,4)];
   [command_string, index] = code_int32(command_string, index, int32(-1));
else
  command_string = [command_string zeros(1,payloadLgth)];
  [command_string, index] = code_float(command_string, index, params.phiSpan);
  [command_string, index] = code_float(command_string, index, params.amplDecay);
  [command_string, index] = code_int8(command_string, index, params.topDetWndSize);
  [command_string, index] = code_float(command_string, index, params.top2SideThr);
  [command_string, index] = code_int8(command_string, index, params.sideGuard);

end


global CRC_ENGINE;
command_string = command_string(1:index-1);
encoding_and_send(board, CRC_ENGINE, command_string);

pause(.01);
stream_in = read_answer_crc(board, CRC_ENGINE);

if (isempty(stream_in)==1)
    ret_code=1;
    return;
end

if (stream_in(1)==COMMAND && (length(stream_in)==(payloadLgth+1)))
    index = 2;
    [index, paramsOut.phiSpan] = get_float(stream_in, index);
    [index, paramsOut.amplDecay] = get_float(stream_in, index);
    [index, paramsOut.topDetWndSize] = get_int8(stream_in, index);
    [index, paramsOut.top2SideThr] = get_float(stream_in, index);
    [index, paramsOut.sideGuard] = get_int8(stream_in, index);
else
    ret_code = 1;
end;


end

