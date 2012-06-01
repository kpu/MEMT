#!/usr/bin/env ruby
require 'pathname'
AVENUE_DIR = File.dirname(Pathname.new(File.expand_path(__FILE__) + '/../../..').realpath)
require AVENUE_DIR + '/MEMT/scripts/zmert/zmert'

throw "Tunes MEMT.  Pass a working directory, connection, and language" unless ARGV[2]
directory=ARGV[0]
connection=ARGV[1]
language=ARGV[2]

full_zmert(directory, connection, language)
