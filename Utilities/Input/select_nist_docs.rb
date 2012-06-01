#!/usr/bin/env ruby
require 'set'

def expect(line)
  got = $stdin.gets.strip
  throw "Expected '#{line}', got '#{got}'" unless got == line
  puts line
end

unless ARGV[0] then
  puts "#{__FILE__} documents <in.xml >out.xml"
  puts "Filters and reorders documents in NIST format XML files.  Put the desired docids in a file one per line then pass that as the only argument."
  exit
end

selected_docs = {}
File.new(ARGV[0]).readlines.each_with_index do |doc, i|
  doc.chomp!
  throw "Duplicate document #{doc}" if selected_docs.key?(doc)
  selected_docs[doc] = i
end

selected_inverse = selected_docs.invert

expect('<?xml version="1.0" encoding="UTF-8"?>')
expect('<!DOCTYPE mteval SYSTEM "ftp://jaguar.ncsl.nist.gov/mt/resources/mteval-xml-v1.3.dtd">')
expect('<mteval>')

FORMATTING=Set.new ["<hl>", "</hl>", "<p>", "</p>"]

#tstset loop
loop do
  setline = $stdin.gets.strip
  puts setline
  if setline == "</mteval>" then
    break
  end
  match = setline.match /^ *<[a-z]*set .*> *$/
  throw "Expected a *set line, got #{setline}" unless match
  docs = Array.new(selected_docs.size)
  #doc loop
  ending = loop do
    doc = $stdin.gets.strip
    next if doc.empty?
    if doc == "</refset>" or doc == "</tstset>" or doc == "</srcset>" then
      break doc
    end
    doc_match = doc.match /^ *<doc +docid="([^"]*)" +genre="(nw|wb)"> *$/
    /"/
    # above line just to get vim back to normal
    throw "Bad doc #{doc}" unless doc_match
    docid = doc_match[1]

    index = selected_docs[docid]
    docs[index] = doc + "\n" if index
    
    # seg loop
    loop do
      seg = $stdin.gets.strip
      docs[index] += seg + "\n" if index
      break if seg == "</doc>"
      if FORMATTING.include?(seg) then
        next
      end
      seg_match = seg.match /^ *<seg id="([0-9]*)">(.*)<\/seg> *$/
      throw "Bad seg line #{seg}" unless seg_match
      throw "Double segment line #{seg}" if seg_match[1].index("<seg")
    end
  end
  docs.each_with_index do |d, i|
    unless d then
      $stderr.puts "Could not find document #{selected_inverse[i]}.  Skipping."
      next
    end
    $stdout.write d
  end
  puts ending
end
