#function wrapper

function [ ret_code, v ] = sg_iterations(board, iterations)
  [ret_code, v] = set_slow_time_gain(board, iterations);
end
