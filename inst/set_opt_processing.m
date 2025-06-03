function [ ret_code, en_dcrem,  en_corr] = set_opt_processing(board, en_dcrem, en_corr, en_matchfilt, en_det_algo, cplx_image, en_rec_algo)

ret_code = 0;

if (en_dcrem)
  en_dcrem =1;
endif
if (en_corr)
  en_corr=1;
endif
if (en_matchfilt)
  en_matchfilt=1;
endif
if (en_det_algo)
  en_det_algo=1;
endif

if (cplx_image)
  cplx_image=1;
endif

if(en_rec_algo)
  en_rec_algo=1;
endif




COMMAND    = uint8('b');
#map (DMREM bit0, ENCORR bit 1)
ui8elab = uint8(en_dcrem + en_corr*2+en_matchfilt*4);
ui8appOpt = uint8(en_det_algo + cplx_image*2 + en_rec_algo*4);
command_string = [  COMMAND zeros(1,16)];

index=2;
[command_string, index] = code_int8(command_string, index, ui8elab);
[command_string, index] = code_int8(command_string, index, ui8appOpt);
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
    [~,v] = get_int8(stream_in,2);
    if (v~=ui8elab)
        ret_code = 1;
    else
        ret_code = 0;
    end;
else
    ret_code = 1;
end;


end

