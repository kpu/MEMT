#!/usr/bin/env ruby
require 'set'
files = ARGV.map { |f| File.new(f) }
loop do
  lines = files.map { |f| f.gets }.compact
  break if lines.empty?
  throw "Unequal number of lines" unless lines.size == files.size
  vocab = Set.new
  lines.each do |l|
    vocab.merge(l.split)
  end
  vocab.each do |w|
    $stdout.write w
    $stdout.write " "
  end
  $stdout.write "\n"
end
