function [ ret_code, en_dcrem, en_corr, en_matchfilt, en_det_algo, cplx_image, en_rec_algo, dis_ao_ant_pol_corr] = set_opt_processing(board, varargin)



ret_code = 0;


en_dcrem = [];
en_corr = [];
en_matchfilt = [];
en_det_algo = [];
cplx_image = [];
en_rec_algo = [];
dis_ao_ant_pol_corr = [];

COMMAND    = uint8('b');
#en_dcrem, en_corr, en_matchfilt, en_det_algo, cplx_image, en_rec_algo
if ((length(varargin) == 6) || (length(varargin) == 7))
  en_dcrem = varargin{1};
  en_corr = varargin{2};
  en_matchfilt = varargin{3};
  en_det_algo = varargin{4};
  cplx_image = varargin{5};
  en_rec_algo = varargin{6};

  if (length(varargin)==7)
    dis_ao_ant_pol_corr = varargin{7}
  else
    dis_ao_ant_pol_corr = 0;
  end


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

  if (dis_ao_ant_pol_corr)
    dis_ao_ant_pol_corr=1;
  endif

  #map (DMREM bit0, ENCORR bit 1)
  ui8elab = uint8(en_dcrem + en_corr*2+en_matchfilt*4);
  ui8appOpt = uint8(en_det_algo + cplx_image*2 + en_rec_algo*4);
  command_string = [  COMMAND zeros(1,16)];

  index=2;
  [command_string, index] = code_int8(command_string, index, ui8elab);
  [command_string, index] = code_int8(command_string, index, ui8appOpt);
  command_string = command_string(1:index-1);
else
  command_string = [  COMMAND ];
end

global CRC_ENGINE;

encoding_and_send(board, CRC_ENGINE, command_string);
pause(.01);
stream_in = read_answer_crc(board, CRC_ENGINE);
if (isempty(stream_in)==1)
    ret_code=1;
    return;
end

if (stream_in(1)==COMMAND)
    [ind,v] = get_int8(stream_in,2);
    [~,v1] = get_int8(stream_in,ind);
    en_dcrem = mod(v, 2);
    en_corr = mod(bitshift(v,-1), 2);
    en_matchfilt = mod(bitshift(v,-2), 2);

    en_det_algo = mod(v1, 2);
    cplx_image = mod(bitshift(v1,-1), 2);
    en_rec_algo = mod(bitshift(v1,-2), 2);
    dis_ao_ant_pol_corr = mod(bitshift(v1,-3), 2);
else
    ret_code = 1;
end;


end

