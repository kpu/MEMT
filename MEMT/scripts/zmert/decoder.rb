#!/usr/bin/env ruby
#Decoder process run by zmert that converts the necessary file formats.
require 'pathname'
AVENUE_DIR = File.dirname(Pathname.new(File.expand_path(__FILE__) + '/../../..').realpath)

require AVENUE_DIR + '/MEMT/scripts/decode'
require AVENUE_DIR + '/MEMT/scripts/zmert/fuzz'
require AVENUE_DIR + '/MEMT/scripts/zmert/format'

iter = ARGV[0].to_i
$stderr.puts "Guessing iteration #{iter}"
config = make_config("dec_cfg.txt", "decoder_config_base") + "\noutput.lowercase = true\n" + Fuzz.new.string_amount(iter)

language = File.new("language").read.strip

decode(config, File.new("connection").read, File.new("dev.matched"), "output", language)
