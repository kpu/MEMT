# Carnegie Mellon University
# Copyright (c) 2009
# All Rights Reserved.
#
# Any use of this software must follow the terms
# outlined in the included LICENSE file.
#

require SCORE_DIR + '/lib/shell_escape'

#Make annoying parentheses after each sentence
def terrify(lines, group_size, output_file_name)
  output_file = File.new(output_file_name, "w")
  #1-index
  index = group_size
  lines.each do |l|
    output_file.write l.strip
    output_file.puts " (" + (index / group_size).to_s + ")"
    index += 1
  end
  output_file.close
end

def score_ter(request)
  hyp_ter = request.output.temp("test.terin")
  terrify(request.hyp.lines, 1, hyp_ter)
  ref_ter = request.output.temp("ref.terin")
  terrify(request.ref.laced_lines, request.ref.number, ref_ter)
  output_file = request.output.perm("ter_out")
  request.output.record_perm(output_file + ".sum")
  args = ["java", "-jar", SCORE_DIR + "/tercom-0.7.25/tercom.7.25.jar", "-r", ref_ter, "-h", hyp_ter, "-N", "-o", "sum", "-n", output_file]
  args << "-s" if request.cased
  system_with_redirect(args, nil, output_file)
  score_line = File.new(output_file).readlines[-4]
  throw "Bad TER output in #{output_file}" unless score_line
  matched = score_line.match(/^Total TER: ([0-9]*.[0-9]*) \([0-9]*.[0-9]*\/[0-9]*.[0-9]*\)$/)
  throw "TER score line \"#{score_line}\" in #{output_file} did not parse" unless matched
  matched[1].to_f
end
