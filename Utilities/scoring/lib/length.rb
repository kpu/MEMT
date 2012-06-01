# Carnegie Mellon University
# Copyright (c) 2009
# All Rights Reserved.
#
# Any use of this software must follow the terms
# outlined in the included LICENSE file.
#

#This computes the average over hypotheses of the ratio of hypothesis length to average corresponding reference length.

def count_words(str)
  str.split(' ').size
end

def score_length(request)
  sum = 0.0
  ref_count = request.ref.number.to_f
  request.hyp.lines.each_index do |i|
    hyp_length = count_words(request.hyp.lines[i])
    ref_sum = 0
    request.ref.range(i).each do |r|
      ref_sum += count_words(request.ref.laced_lines[r])
    end
    sum += (hyp_length.to_f / (ref_sum.to_f / ref_count))
  end
  sum / request.hyp.lines.size.to_f
end
