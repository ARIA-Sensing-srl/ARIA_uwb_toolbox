% *************************************************
% Aria Sensing srl 2024
% Confidential-reserved
% *************************************************

#return array with range and azimuth (N, 2) or [] if no detection, type is optional parameter for detection level (0: blob, 1 filtered)
function [ ret_code,detOut, detFullState] = get_detection_2D(board, type)


COMMAND    = uint8('E');
detOut = [];
detFullState = [];
ret_code = 0;

command_string = [  COMMAND ];


if (isempty(type) == 0)
  index=2;
  command_string = [command_string uint8(zeros(1,1))];
  [command_string, index] = code_int8(command_string, index, int8(type));
else
  type = 0; #defalt type blobs
end

bytesPerPoint = 8;
if (type ~= 0)
  bytesPerPoint = 10; #position + trackID
endif

switch (type)
  case 0
    #blobs
    bytesPerPoint = 8;
  case 1
    #track position not filtered
    bytesPerPoint = 10;
  case 2
    #track position filtered
    bytesPerPoint = 10;
  case 3
    #track SV+P not filtered
    bytesPerPoint = 20*4+2;
  case 4
    #track SV+P filtered
    bytesPerPoint = 20*4+2;
  otherwise
    rc=1;
endswitch

global CRC_ENGINE;

encoding_and_send(board, CRC_ENGINE, command_string);

pause(.01);
stream_in = read_answer_crc(board, CRC_ENGINE);
if (isempty(stream_in)==1)
    ret_code=1;
    return;
end

% Read answer
if (isempty(stream_in)==0)
    if (stream_in(1)==COMMAND)
        if (length(stream_in)>1)
          stream_in = stream_in(2:end);
          if (mod(length(stream_in),bytesPerPoint) ~= 0)
            ret_code = 1; #malformed
            return;
          endif
          Nblobs = length(stream_in)/bytesPerPoint;

          if (type == 0)
            detOut = zeros(Nblobs, 2);
            index = 1;
            for N = 1:Nblobs
              [index, detOut(N,1)] = get_float(stream_in, index);
              [index, detOut(N,2)] = get_float(stream_in, index);
            endfor
          elseif (type == 1 || type==2)
            #position of the track
            detOut = zeros(Nblobs, 3);
            index = 1;
            for N = 1:Nblobs
              [index, detOut(N,1)] = get_float(stream_in, index);
              [index, detOut(N,2)] = get_float(stream_in, index);
              [index, detOut(N,3)] = get_int16(stream_in, index);
            endfor
          else
            #position + sv + P + ID
            detOut = zeros(Nblobs, 3);
            index = 1;
            for N=1:Nblobs
              curSV = zeros(4,1);
              for t = 1:4
                [index, curSV(t)] = get_float(stream_in, index);
              endfor
              curP = zeros(1,4);
              for t = 1:16
                [index, curP(t)] = get_float(stream_in, index);
              endfor
              curP = reshape(curP, 4, 4)';
              [index, curID] = get_int16(stream_in, index);

              detFullState(N).StateVector = curSV;
              detFullState(N).P = curP;
              detFullState(N).ID = curID;

              #populate detection out
              detOut(N, 1:2) = curSV([1 3])';
              detOut(N, 3) = curID;
            endfor
          endif
        endif
    else
        ret_code = 1;
        return;
    end;
else
    ret_code=1;
end



end

