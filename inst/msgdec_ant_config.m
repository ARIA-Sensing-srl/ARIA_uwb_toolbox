function [retRxList, retTxList]= msgdec_ant_config (stream_in)
  retRxList = [];
  retTxList = [];
  IDTxEncodeOffset = 4;
  index = 1;
  #expected size is a multiple of 17 (command excluded)

  if (mod(length(stream_in), 17) ~= 0)
    return;
  endif
  numItems = (length(stream_in))/17;

  for N = 1:numItems
    [index, curData.ID] = get_int8(stream_in, index);
    [index, curData.X] = get_float(stream_in, index);
    [index, curData.Y] = get_float(stream_in, index);
    [index, curData.delay] = get_float(stream_in, index);
    [index, curData.ampl] = get_float(stream_in, index);
    if (curData.ID >= (IDTxEncodeOffset) )
      curData.ID = curData.ID-IDTxEncodeOffset;
      retTxList = [retTxList curData];
    else
      retRxList = [retRxList curData];
    endif
  endfor
endfunction
