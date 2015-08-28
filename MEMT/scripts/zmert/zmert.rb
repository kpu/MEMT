#!/usr/bin/env ruby
require AVENUE_DIR + '/MEMT/scripts/decode'
require AVENUE_DIR + '/MEMT/scripts/shell_escape'
require AVENUE_DIR + '/MEMT/scripts/util'
require AVENUE_DIR + '/MEMT/scripts/zmert/fuzz'
require AVENUE_DIR + '/MEMT/scripts/zmert/format'

def get_file(directory, name)
  full = directory + '/' + name
  throw "Missing #{full}" unless File.exists?(full)
  full
end

def write_params(directory, features)
  out = File.new(directory + "/params.txt", "w")
  #TODO: more rigorous treatement of features instead of special case for length
  features.each_with_index do |sign, idx|
    out.puts "param#{idx} ||| 0.0 Opt " + if sign > 0 then
      "0.0 +Inf 0.0 1.0"
    elsif sign == 0 then
      "-Inf +Inf -1.0 1.0"
    else
      "-Inf 0.0 -1.0 0.0"
    end
  end
  out.puts "normalization = LNorm 1 1"
  out.close
end

def write_feature_config(directory, features)
  out = File.new(directory + '/dec_cfg.txt', "w")
  features.size.times do |i|
    out.puts "param#{i} 0.0"
  end
  out.close
end

def count_sentences(matched)
  i = 0
  while matched.get_match do
    i += 1
  end
  i
end

#Because Omar wrongly decided to default to one CPU.  
closeme=IO.popen(AVENUE_DIR + '/bin/print_concurrency')
response=closeme.read.to_i
closeme.close
CONCURRENCY = (response==0 ? 1 : response)

def write_zmert_config(directory, zmert_args, refs_per_sentence, nbest)
  write_close(directory + '/zmert_config.txt',
"-rps #{refs_per_sentence}
-N #{nbest}
-cmd #{AVENUE_DIR}/MEMT/scripts/zmert/decoder.rb
-passIt 1
-thrCnt #{CONCURRENCY}
-minIt #{Fuzz.slide_amount}
#{zmert_args}
")
end

LOWERCASER_DIR=AVENUE_DIR + "/bin"
throw "Lowercaser is not executable.  Compile it in #{AVENUE_DIR}/util" unless File.executable?(LOWERCASER_DIR + '/lower')
append_colon_environment("LD_LIBRARY_PATH", LOWERCASER_DIR)
def lowercase_file(from, to)
  system_with_redirect([LOWERCASER_DIR + "/lower"], from, to)
end

def prepare_working(directory, connection, language)
  throw "Working directory '#{directory}' does not exist" unless File.exists?(directory)

  matched = JavaMatched.new(File.new(get_file(directory, 'dev.matched')))
  systems = matched.sys_count

  base_config = File.new(get_file(directory, 'decoder_config_base')).read
  features = feature_info(connection, base_config, systems)

  reference = get_file(directory, 'dev.reference')

  sentence_count = count_sentences(matched)
  reference_count = count_lines(File.new(reference))
  throw "Have #{reference_count} references for #{sentence_count} sentences" unless reference_count % sentence_count == 0
  refs_per_sentence = reference_count / sentence_count

  zmert_work = directory + "/zmert"
  Dir.mkdir(zmert_work)

  metric = if File.exists?(directory + "/metric") then
    File.new(directory + "/metric").read
  else
    "BLEU 4 closest"
  end
  zmert_args = "-m #{metric}\n" + if File.exists?(directory + "/zmert_config_base") then
    File.new(directory + "/zmert_config_base").read
  else
    "-decV 1\n-v 1"
  end

  #params.txt
  write_params(zmert_work, features)
  #dec_cfg.txt
  write_feature_config(zmert_work, features)

  nbest = base_config.match(/^\s*output.nbest\s*=\s*([0-9]*)\s*$/)
  throw "Couldn't parse output.nbest in #{base_config}" unless nbest
  #zmert_config.txt
  write_zmert_config(zmert_work, zmert_args, refs_per_sentence, nbest[1])
  #connection
  write_close(zmert_work + '/connection', connection)
  write_close(zmert_work + '/language', language)
  #decoder_config_base
  File.symlink("../decoder_config_base", zmert_work + '/decoder_config_base')
  #dev.matched
  File.symlink("../dev.matched", zmert_work + '/dev.matched')
  #reference.txt
  lowercase_file(reference, zmert_work + "/reference.txt")
end

def run_zmert(directory)
  Dir.chdir(directory + '/zmert') do
    #the original "-maxMem" of Joshua ZMERT is 1000, if your RAM is large enough, I strongly suggested to set it as large as possible 
    command = escape_shell_array(["java", "-Xms1G", "-Xmx3G", "-cp", AVENUE_DIR + "/Utilities/scoring/meteor-1.0/dist/meteor-1.0/meteor.jar:" + AVENUE_DIR + "/Utilities/Tuning/zmert.jar", "joshua.zmert.ZMERT","-maxMem", "1000", "zmert_config.txt"])
    $stdout.puts "Running #{command}"
    #z-mert is buggy wrt , versus . otherwise
    oldenv = ENV["LC_ALL"]
    ENV["LC_ALL"] = "C"
    begin
      throw "Command '#{command}' failed" unless system(command)
    ensure
      ENV["LC_ALL"] = oldenv
    end
  end
end

def write_finished_config(directory)
  write_close(directory + '/decoder_config', make_config(directory + '/zmert/dec_cfg.txt.ZMERT.final', directory + '/decoder_config_base'))
end

def full_zmert(directory, connection, language)
  prepare_working(directory, connection, language)
  run_zmert(directory)
  write_finished_config(directory)
  #TODO: optimize case where MERT leaves parameters unchanged and fuzz is off.  
  decode(File.new(directory + "/decoder_config").read, connection, File.new(directory + "/dev.matched"), directory + '/output', language)
  system(escape_shell_array([AVENUE_DIR + "/Utilities/scoring/score.rb", "--language", language.split('_')[0], "--task", "af", "--hyp-detok", directory + "/output.1best", "--refs-laced", directory + "/dev.reference"]))
end
