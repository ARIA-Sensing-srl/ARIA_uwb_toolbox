function [] = encoding_and_send(board,crcEngine, stream)
%UNTITLED4 Summary of this function goes here
%   Detailed explanation goes here
global DEFINE_OCTAVE;
START_CHAR = 0xFF;
END_CHAR   = 0x00;

streamLgth = length(stream);

if (isempty(crcEngine)==0)
    crc=calcCrc8(stream, crcEngine);
    %append crc to stream
    stream =[stream crc];
    streamLgth = streamLgth+1;
end

streamOut=[];
for N=1:streamLgth
    curChar = stream(N);
    switch (curChar)
        case 0xFF
            %replace start with alternate sequence 0x80 0xFE
            streamOut = [streamOut 0x80 0xFE];
        case 0x00
            %replace stop with alternate sequence 0x80 0x01
            streamOut = [streamOut 0x80 0x01];
        case 0x80
            %replace with 0x80 0x81
            streamOut = [streamOut 0x80 0x81];
        otherwise
            %copy data
            streamOut =  [streamOut curChar];

    end
end

streamOut = [START_CHAR streamOut END_CHAR];

if (DEFINE_OCTAVE==0)
	flushinput(board);
	fwrite(board,streamOut);
else
	srl_flush(board);
	srl_write(board,streamOut);
##  srl_flush(board);
end;

end

