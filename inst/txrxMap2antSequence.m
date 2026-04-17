## Copyright (C) 2026 Andrea Mario
##
## This program is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program.  If not, see <https://www.gnu.org/licenses/>.

## -*- texinfo -*-
## @deftypefn {} {@var{scan_sequence} =} txrxMap2antSequence (@var{rxMask}, @var{txMask})
## Return @var{scan_sequence} (encoded as [txch_stream0 rxch_stream0; etc.] one row per stream
## @var{rxMask} and @var{txMask} are the encoded sequences get from the device
## @seealso{}
## @end deftypefn

## Author: Andrea Mario <andream@localhost>
## Created: 2026-04-15

function [scan_sequence]= txrxMap2antSequence (rxMask, txMask)
  scan_sequence = [];
  if (size(rxMask, 1) != 1)
    rxMask = transpose(rxMask);
  endif

  if (size(txMask, 1) != 1)
    txMask = transpose(txMask);
  endif

  if (sum(size(rxMask) != size(txMask)))
    error "rxMask txMask dimension mismatch"
  endif

  scan_sequence = log2([txMask' rxMask']);

  #check
  frac = scan_sequence - fix(scan_sequence);
  if (sum(any(frac) != 0))
    error "Invalid input mapping"
  endif

endfunction
