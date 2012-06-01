# Carnegie Mellon University
# Copyright (c) 2009
# All Rights Reserved.
#
# Any use of this software must follow the terms
# outlined in the included LICENSE file.
#

require SCORE_DIR + '/lib/shell_escape'

def parse_meteor(line, expression)
  matched = line.match(expression)
  throw "Meteor line #{line.inspect} does not match #{expression}." unless matched
  matched[1].to_f
end

def score_meteor(request)
  output_file = request.output.perm("meteor_out")
  system_with_redirect(["java", "-jar", SCORE_DIR + "/meteor-1.0/dist/meteor-1.0/meteor.jar", request.hyp.file_name, request.ref.laced_name, "-r", request.ref.number.to_s, "-normalize", "-l", request.language, "-t", request.task], nil, output_file)
  score_lines = File.new(output_file).readlines
  throw "Meteor output should be at least 7 lines" unless score_lines.size > 7
  [parse_meteor(score_lines[-1], /^Final score:\t\t([0-9]*\.[0-9]*)$/), parse_meteor(score_lines[-7], /^Precision:\t\t([0-9]*\.[0-9]*)$/), parse_meteor(score_lines[-6], /^Recall:\t\t\t([0-9]*\.[0-9]*)$/)].map do |n|
    n.to_f
  end
end
