#!/usr/bin/env ruby
#Standalone program to interlace multiple files.  Useful to make files for --refs-laced
files = ARGV.map { |f| File.new(f) }
loop do
  lines = files.map { |f| f.gets }.compact
  break if lines.empty?
  throw "Unequal number of lines" unless lines.size == files.size
  lines.each do |l|
    $stdout.puts l
  end
end
