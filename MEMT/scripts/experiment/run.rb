#!/usr/bin/env ruby
require 'pathname'
AVENUE_DIR = File.dirname(Pathname.new(File.expand_path(__FILE__) + '/../../..').realpath)

require AVENUE_DIR + "/MEMT/scripts/zmert/zmert"

require 'fileutils'

def symlink_rel(src, copy)
  FileUtils.ln_s("../" * (copy.split('/').size - 1) + src, copy, :force => true)
end

def record_run(name, to)
  Dir.mkdir(to + '/runs') unless File.exists?(to + '/runs')
  symlink_rel('runs/' + name, to + '/runs/' + name)
end

class AlreadyExists < Exception
  attr_reader :work_dir
  def initialize(work_dir)
    @work_dir = work_dir
  end
  def to_s
    @work_dir
  end
end

def setup_pair(config, matched, additional = "")
  name = config.split('/')[1..-1].join('-') + "_" + matched.split('/')[1..-1].join('-')
  name += "_" + additional if additional != ""
  work_dir = "runs/" + name
  raise AlreadyExists, work_dir if File.exists?(work_dir)
  $stderr.puts work_dir
  Dir.mkdir(work_dir)
  symlink_rel(matched, work_dir + '/matched')
  symlink_rel(config, work_dir + '/config')
  Dir.foreach(config).each do |ent|
    next if ent == ".." or ent == "."
    FileUtils.ln_sf("config/" + ent, work_dir + '/' + ent)
  end
  FileUtils.ln_sf("matched/tune/matched", work_dir + "/dev.matched")
  reference = matched.split('/')[0..-3].join('/') + '/ref'
  symlink_rel(reference, work_dir + "/ref")
  FileUtils.ln_sf("ref/tune/laced", work_dir + "/dev.reference")

  record_run(name, config)
  record_run(name, matched)
  work_dir
end

def run_pair(connection, config, matched, additional = "")
  language = File.new(matched + '/../../language').read.chomp
  work_dir = setup_pair(config, matched, additional)
  $stdout.puts "Starting experiment " + work_dir
  full_zmert(work_dir, connection, language)
  Dir.mkdir(work_dir + "/eval")
  config = File.new(work_dir + '/decoder_config').read
  config.sub!(/output\.nbest *= *[0-9]*/, "output.nbest = 1") unless identify_microsoft(config, false)[0]
  decode(config, connection, File.new(work_dir + "/matched/eval/matched"), work_dir + "/eval/output", language)
  
  system(AVENUE_DIR + "/Utilities/scoring/score.rb", "--language", language.split('_')[0], "--task", "af", "--hyp-detok", work_dir + "/eval/output.1best", "--refs-laced", work_dir + "/ref/eval/laced")
end

connection = ARGV[0]
configs = []
matched = []
additional = ""
ARGV[1..-1].each do |arg|
  first = arg.split('/')[0]
  if first == "config" then
    configs << arg
  elsif first == "corpus"
    matched << arg
  elsif first == "additional"
    additional = arg.split('/')[1]
  else
    throw "Confused by argument " + arg
  end
end

configs.each do |c|
  matched.each do |m|
    begin
      run_pair(connection, c, m, additional)
    rescue AlreadyExists => e
      $stderr.puts "Skipping #{e.work_dir} since it already exists"
    end
  end
end
