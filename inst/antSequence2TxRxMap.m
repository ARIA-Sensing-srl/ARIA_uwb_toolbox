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
## @deftypefn {} {@var{rxMask}, @var{txMask} =} antSequence2TxRxMap (@var{scan_sequence})
## Encode antenna sequence to be sent to the device
## @var{scan_sequence}: nx2 array, every raw set an antenna pair (tx rx) encoded from 0 to 3 (Hydrogen channel)
##
## @seealso{}
## @end deftypefn

## Author: Andrea Mario <andream@localhost>
## Created: 2026-04-15

function [rxMask, txMask]= antSequence2TxRxMap (scan_sequence)
  rxMask=[];
  txMask=[];

  if (size(scan_sequence,1) > 16)
    error("Max supported sequence are 16\n");
  endif

  if (size(scan_sequence,2) != 2)
    error("Input array must have 2 columns\n");
  endif

  scan_sequence = round(scan_sequence);

  if (sum(any(scan_sequence > 3)) || sum(any(scan_sequence < 0)))
    error("Invalid encoding\n");
  endif

  rxMask = 2.^scan_sequence(:, 2);
  txMask = 2.^scan_sequence(:, 1);

endfunction
