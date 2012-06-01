#!/usr/bin/env ruby

# Carnegie Mellon University
# Copyright (c) 2009
# All Rights Reserved.
#
# Any use of this software must follow the terms
# outlined in the included LICENSE file.
#

require 'set'
require 'pathname'
SCORE_DIR = File.dirname(Pathname.new(File.expand_path(__FILE__)).realpath)
require SCORE_DIR + '/lib/nistify'
require SCORE_DIR + '/lib/terrify'
require SCORE_DIR + '/lib/meteorify'
require SCORE_DIR + '/lib/length'
require SCORE_DIR + '/lib/shell_escape'

def get_num_refs(hyp, refs, expected_multiplier)
  hyp_lines = hyp.size
  refs_lines = refs.size
  throw "Number disagreement: #{hyp_lines} hypotheses and #{refs_lines} references" if refs_lines % hyp_lines != 0
  num_refs = refs_lines / hyp_lines
  throw "Expected #{expected_multiplier} reference per sentence, got #{num_refs}" if expected_multiplier and  num_refs != expected_multiplier
  num_refs
end

class Output
  def initialize(prefix, delete_temp, delete_perm)
    @prefix = prefix.clone
    if File.directory?(@prefix) then
      @prefix += '/' if  @prefix[-1] != '/'[0]
    else
      @prefix += '.' if File.exists?(@prefix)
    end
    @delete_temp = delete_temp
    @delete_perm = delete_perm
    @temps = []
    @perms = []
  end
  #Name a temporary file that will be deleted if @delete_temp
  def temp(name)
    ret = @prefix + name
    @temps << ret
    ret
  end
  #Name a permanent file
  def perm(name)
    ret = @prefix + name
    @perms << ret
    ret
  end
  def record_perm(full_name)
    @perms << full_name
  end

  def finish(print_results)
    $stdout.puts File.new(@prefix + "scores").read if print_results
    if @delete_temp then
      @temps.each do |t|
        File.unlink(t)
      end
    end
    if @delete_perm then
      @perms.each do |p|
        File.unlink(p)
      end
    end
  end
end

class Hypothesis
  attr_reader :file_name, :lines
  def initialize(file_name, tokenized, output)
    if tokenized then
      @file_name = output.temp("hyp-detok")
      system_with_redirect([SCORE_DIR + "/lib/postprocess.pl"], file_name, @file_name)
    else
      @file_name = file_name
    end
    @lines = File.new(@file_name).readlines
  end
end

class Reference
  attr_reader :laced_name, :laced_lines, :number
  def initialize(type, arg, hyp_count, output)
    if type == :single then
      @number = 1
      @laced_name = arg
      @laced_lines = File.new(@laced_name).readlines
    elsif type == :laced then
      @laced_name = arg
      @laced_lines = File.new(@laced_name).readlines
    elsif type == :multiple then
      @laced_name = output.temp("ref.laced")
      @number = arg.size
      laced_file = File.new(@laced_name, "w")
      files = arg.map do |ref|
        File.new(ref)
      end
      @laced_lines = []
      loop do
        lines = files.map { |f| f.gets }
        compact = lines.compact
        break if compact.size == 0
        throw "Unequal number of lines" if compact.size != files.size
        lines.each do |l|
          l.strip!
          @laced_lines << l
          laced_file.puts l
        end
      end
      laced_file.close
    else
      throw "Bad type #{type}"
    end
    throw "There are #{hyp_count} hypotheses and #{laced_lines.size} total references" unless laced_lines.size % hyp_count == 0
    number = laced_lines.size / hyp_count
    throw "Size of references is #{number} times number of hypotheses, but #{@number} specified" if @number and @number != number
    @number = number
  end
  def range(segment)
    ((segment * number)..(segment * number + number - 1))
  end
end

class ScoreRequest
  attr_reader :output, :hyp, :ref, :cased, :language, :task
  def initialize(output, hyp, ref, cased, language, task)
    @output = output
    @hyp = hyp
    @ref = ref
    @cased = cased
    @language = language
    @task = task
  end
end

def die_on_exception
  begin
    yield
  rescue => error
    $stderr.puts error.backtrace
    $stderr.puts error
    exit 1
  end
end

def thread_die_on_exception
  Thread.new do die_on_exception do yield end end
end

#Takes a ScoreRequest
def score(request)
  nist = nil
  ter = nil
  meteor = nil
  length = nil
  threads = []
  threads << thread_die_on_exception do
    nist = score_nist(request)
  end
  threads << thread_die_on_exception do
     ter = score_ter(request)
  end
  threads << thread_die_on_exception do 
    meteor = score_meteor(request)
  end
  threads << thread_die_on_exception do
    length = score_length(request)
  end
  threads.each do |t|
    t.join
  end
  #BLEU NIST TER METEOR METEOR_Precision METEOR_Recall Length
  scores = [nist[1] * 100.0, nist[0], ter * 100.0, meteor[0] * 100.0, meteor[1] * 100.0, meteor[2] * 100.0, length]
  formats = ["%5.2f", "%5.2f", "%5.2f", "%5.2f", "%5.2f", "%5.2f", "%0.02f"]
  scores_string = []
  scores.each_index do |i|
    scores_string << sprintf(formats[i], scores[i])
  end
  scores_string = scores_string.join(' ')
  score_out = File.new(request.output.perm("scores"), "w")
  score_out.puts(scores_string)
  score_out.close
end

def parse_args(names)
  var = nil
  parsed = {}
  process = true
  ARGV.each do |arg|
    if arg[0..1] == "--" and process then
      name = arg[2..-1]
      #-- ends argument processing
      if name == "" then
        process = false
        next
      end
      throw "Bad argument #{arg}" unless names.include?(name)
      throw "Duplicate argument #{arg}" if parsed[name]
      var = Array.new
      parsed[name] = var
      next
    end
    throw "First argument must begin with --" unless var
    var << arg
  end
  parsed
end


HEADER = "BLEU  NIST  TER   METEO Preci Recal Leng"
USAGE=__FILE__ + "
--hyp-[de]tok hyp.txt [--ref ref.txt] [--refs ref1.txt ref2.txt]
[--refs-laced ref-interlaced.txt]
[--output output_prefix] [--print-header] [--print] [--keep-temp]
[--delete-results]
Scores using BLEU NIST TER METEOR METEOR_Precision METEOR_Recall Length in that order.
Aligned to output spacing, these scores are:
#{HEADER}

HYPOTHESIS
The hypothesis file is plaintext one sentence per line with no HTML character entities.
--hyp-tok means output is piped through Greg Hanneman postprocess.pl first.
--hyp-detok means only leading and trailing spaces are removed.  

REFERENCE
Specify the reference using exactly one of --ref, --refs, or --refs-laced.  
All reference files are plaintext with no HTML character entities.
--ref means a single reference, one per line.  Same as --refs except this
  checks for one argument.  Also the same as --refs-laced  except this checks
  for one reference.
--refs means multiple reference files are specified, where the nth line of each
  is a reference for line n of the hypothesis.
--refs-laced means that the references for the first segment appear at the top
  followed by references for the second segment, etc.  Further, the number of
  references for each sentence is constant.  This is the reference format
  accepted by Meteor. 

PARAMETERS
--cased activates case sensitivity for BLEU, NIST, and TER.  METEOR is not impacted.
  The default is case insensitive.
--language sets the language, as accepted by METEOR.  BLEU and NIST get this
  set in the SGML field, which appears to have no impact on scoring.  TER
  doesn't care. 
--task is the task for METEOR: af, rank, or hter.  Default is hter.  

OUTPUT
--output specifies a prefix for output.  This defaults to the hypothesis file
  with a period appended.  If a directory is specified, a / is appended if it
  does not appear already.
--print-header prints the column headers.  This may be used alone.  
--print prints the scores to stdout.
--keep-temp will leave temporary files such as bogus NIST SGML files around.
--delete-results deletes result files, in which case you probably want --print too.
"

if ARGV.empty? then
  $stdout.puts USAGE
  exit 0
end

begin
  parsed = parse_args(Set.new(["hyp-tok", "hyp-detok", "ref", "refs", "refs-laced", "output", "cased", "keep-temp", "delete-results", "print-header", "print", "language", "task"]))
  ["cased", "print-header", "print", "keep-temp", "delete-results"].each do |name|
    throw "Expected no argument for #{name}" if parsed[name] and parsed[name].size != 0
  end
  #Special case: print-header may be called alone.  
  if parsed.key?("print-header") then
    $stdout.puts HEADER
    exit 0 if parsed.size == 1
  end
  ["hyp-tok", "hyp-detok", "ref", "refs-laced", "output", "language", "task"].each do |name|
    throw "Expected one argument for #{name}" if parsed[name] and parsed[name].size != 1
  end
  throw "Specify one of hyp-tok or hyp-detok" unless [parsed["hyp-tok"], parsed["hyp-detok"]].compact.size == 1
  throw "Specify one of ref, refs, or refs-laced" unless [parsed["ref"], parsed["refs"], parsed["refs-laced"]].compact.size == 1
rescue NameError => bang
  $stderr.puts USAGE
  $stderr.puts
  $stderr.puts "ERROR with your command:"
  $stderr.puts bang.name
  exit 2
end

hypothesis_file = (parsed["hyp-tok"] || parsed["hyp-detok"])[0]
unless File.exists?(hypothesis_file) then
  $stderr.puts "No such file #{hypothesis_file}"
  exit 1
end
output = Output.new(parsed.key?("output") ? parsed["output"][0] : hypothesis_file, (not parsed.key?("keep-temp")), parsed.key?("delete-results"))

File.new(output.perm("args"), "w").puts(escape_shell_array([__FILE__] + ARGV))

hyp = Hypothesis.new(hypothesis_file, parsed.key?("hyp-tok"), output)
ref = if parsed.key?("ref") then
  Reference.new(:single, parsed["ref"][0], hyp.lines.size, output)
elsif parsed.key?("refs-laced") then
  Reference.new(:laced, parsed["refs-laced"][0], hyp.lines.size, output)
else
  Reference.new(:multiple, parsed["refs"], hyp.lines.size, output)
end
language = parsed.key?("language") ? parsed["language"][0] : "en"
task = parsed.key?("task") ? parsed["task"][0] : "hter"
score(ScoreRequest.new(output, hyp, ref, parsed.key?("cased"), language, task))

output.finish(parsed.key?("print"))
