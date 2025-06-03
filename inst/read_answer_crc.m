% *************************************************
% Cover Sistemi srl 2018
% Confidential-reserved
% *************************************************
function [ stream_out, resOut ] = read_answer_crc( board, crcEngine, varargin )
% Read the USB answer from radar module, decode data and check CRC
global DEFINE_OCTAVE;
global CRC_ERROR_CNT;
%stream_out = zeros(1,9192);

START_CHAR = 0xFF;
END_CHAR   = 0x00;

if (nargin == 2)
    rxBuf=[];
    initRxNotEmpty=0;
else
    rxBuf=cell2mat(varargin(1));
    if (isempty(rxBuf)==1)
        initRxNotEmpty=0;
    else
        initRxNotEmpty=1;
    end
end

stream_out=[];
resOut=[];

%output_data=[];
maxTimeout = 0.5;

tic; %for timeout
while(1)
    if (DEFINE_OCTAVE==0)
      bytesAvailable = board.BytesAvailable;
    else
      %[indata, bytesAvailable] = srl_read(board, 128);
      bytesAvailable = board.bytesavailable;
    end
    if (bytesAvailable> 0 || initRxNotEmpty)
        
        %check for start byte
        tic;
        if (bytesAvailable > 0)
            if (DEFINE_OCTAVE==0)
              indata = fread(board,bytesAvailable);
            else
              indata = fread(board,bytesAvailable)';
            end
            rxBuf = [rxBuf ; indata];
        else
            initRxNotEmpty = 0;
            
        end
        numRxBytes=length(rxBuf);
        
        %check start and stop positions
        iStart=find(uint8(rxBuf)==START_CHAR, 1, 'first');
        iEnd=find(uint8(rxBuf)==END_CHAR, 1, 'first');
        
        if (isempty(iStart)==0 && isempty(iEnd)==0)
            break;
        else
            if (toc > maxTimeout)
                %output_data=[];
                return;
            end
        end
    else
        if (toc > maxTimeout)
            %output_data=[];
            return;
        end
    end
end

if (iEnd <= iStart) 
    return;
end
if (length(rxBuf) > iEnd)
    resOut = rxBuf(iEnd+1:end);
end

payload = rxBuf((iStart+1):(iEnd-1)); %remove start and stop

payloadSize=length(payload);
checkChar=0;
for N = 1:payloadSize
    curChar = payload(N);
    if (curChar == 0x80)
        checkChar=1;
        continue;
    else
        if (checkChar)
            checkChar=0;
            switch(curChar)
                case 0xFE
                    stream_out = [stream_out 0xFF];
                case 0x01
                    stream_out = [stream_out 0x00];
                case 0x81
                    stream_out = [stream_out 0x80];
                otherwise
                    stream_out = [];
                    return;
            end
        else
            stream_out = [stream_out curChar];
        end
    end
end

crc=0;
if (isempty(crcEngine)==0)
    crc = calcCrc8(stream_out, crcEngine);
    stream_out = stream_out(1:end-1);
    if (crc ~=0)
        CRC_ERROR_CNT = CRC_ERROR_CNT+1;
    end
end

%check char without counterpart
if(checkChar==1 || crc ~=0)
    stream_out=[];
end
 
end

