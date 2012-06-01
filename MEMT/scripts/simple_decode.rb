#!/usr/bin/env ruby
require 'pathname'
AVENUE_DIR = File.dirname(Pathname.new(File.expand_path(__FILE__) + '/../..').realpath)
require AVENUE_DIR + '/MEMT/scripts/decode'
require AVENUE_DIR + '/MEMT/scripts/zmert/format'

unless ARGV[2]
  $stderr.puts "Usage: connection config_file matched_file [output_prefix] [language for detokenization]"
  exit 1
end

CONN=ARGV[0]
CONFIG=ARGV[1]
MATCHED=ARGV[2]
OUT_BASE=(ARGV[3] ? ARGV[3] : MATCHED)
decode(
	File.new(CONFIG).read,
	CONN,
	File.new(MATCHED, 'r'),
	OUT_BASE,
  ARGV[4])
