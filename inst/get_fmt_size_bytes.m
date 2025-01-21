% *************************************************
% Aria Sensing srl 2024
% Confidential-reserved
% *************************************************

function [retval ,typecast_str] = get_fmt_size_bytes (fmt)
retval = 0;
typecast_str = '';
switch (fmt)
  case 0
    %Q7
    retval = 1;
    typecast_str = 'int8';
  case 1
    %Q15
    retval = 2;
    typecast_str = 'int16';
  case 2
    %Q31
    retval = 4;
    typecast_str = 'int32';
  case 3
    %F32
    retval = 4;
    typecast_str = 'single';
  case 4
    %F16
    retval = 2;
    typecast_str = 'halfprecision';
  case 5
    %F16
    retval = 4;
    typecast_str = 'halfprecision_cplx';
end


endfunction
