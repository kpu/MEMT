#!/usr/bin/env ruby
require 'set'

IGNORED_SEGMENTS=Set.new ["<hl>", "</hl>", "<p>", "</p>", "<TEXT>", "</TEXT>", "<GALE_P2 id=\"S1\">", "<GALE_P2 id=\"S3\">", "<GALE_P3 id=\"S2\">", "</GALE_P2>", "<GALE_P3 id=\"S1\">", "</GALE_P3>", "<GALE_P4 id=\"S1\">", "<GALE_P4 id=\"S2\">", "<GALE_P4 id=\"S3\">", "<GALE_P4 id=\"S4\">", "</GALE_P4>", "<BODY>", "</BODY>", "<POST>", "</POST>"]
THREE_LINE_SEGMENTS=Set.new ["<HEADLINE>", "<DATELINE>", "<KEYWORD>"]

docids = File.new(ARGV[0]).readlines.map { |d| d.strip }
doc_position = {}
docids.each_with_index do |d,i|
  doc_position[d] = i
end


out = []
while line = $stdin.gets
  line.strip!
  next if line.match /<\/?tstset.*>/
  next if ['<!DOCTYPE mteval SYSTEM "ftp://jaguar.ncsl.nist.gov/mt/resources/mteval-xml-v1.0.dtd">', '<!DOCTYPE MTEVAL SYSTEM "http://www.nist.gov/speech/tests/mt/2008/doc/mteval.dtd" []>', '<mteval>', '</mteval>', '<!DOCTYPE mteval SYSTEM "ftp://jaguar.ncsl.nist.gov/mt/resources/mteval-xml-v1.3.dtd">', '<?xml version="1.0" encoding="UTF-8"?>', ''].include?(line)
  docid = if ( matched = line.match /^<(doc|DOC) docid="([^"]*)" *((genre|GENRE)="[^"]*"|) *(sysid="[^"]*"|).*>$/)  
#    /\)"\)\)\)\)/
    #above line to unconfuse vim
    matched[2]
  elsif (matched = line.match /^<(doc|DOC) *(sysid="[^"]*"|) *docid="([^"]*)" *((genre|GENRE)="[^"]*"|).*>$/)
#    /\)")\)\)\)/
    #above line to unconfuse vim
    matched[3]
  elsif (matched = line.match /^<(doc|DOC)  *id="([^"]*)"[^<]*>$/)
#    /\)")\)\)\)/
    matched[2]
  elsif line == "<DOC>"
    line = $stdin.gets
    matched = line.match /<DOCID> *([^< ]*) *<\/DOCID>/
    throw "Bad secondary docid #{line}" unless matched
    matched[1]
  else
    throw "Bad document '#{line}'"
  end
  position = doc_position[docid]
  out[position] = [line] if position
  while line = $stdin.gets
    line.strip!
    if THREE_LINE_SEGMENTS.include?(line)
      loop do 
        line = $stdin.gets
        break if line.include?("<")
      end
      expected = line.insert(1,"/")
      throw "Expected #{expected}; got #{got}" unless line == expected
      next
    end
    next if line.match /^<skipseg id="[0-9]*">.*<\/skipseg>$/
    next if line.match /^<HEADLINE>[^<]*<\/HEADLINE>$/
    next if line.match /^<POSTER>[^<]*<\/POSTER>$/
    next if line.match /^<POSTDATE>[^<]*<\/POSTDATE>$/
    next if line.match /^<DOCTYPE SOURCE="[^"]*">[^<]*<\/DOCTYPE>$/
#    /"/
    next if line.match /^<DATETIME>[^<]*<\/DATETIME>$/
    if line == "</DOC>" || line == "</doc>" then
      out[position] << line if position
      break
    end
    matched = line.match /^<seg id="[^"]*">.*<\/seg>$/
    /"/
    throw "Bad segment '#{line}'" if matched == nil and not IGNORED_SEGMENTS.include?(line)
    out[position] << line if position
  end
end

out.each_with_index do |o,i|
  throw "Did not find docid #{docids[i]}" unless o
  o.each do |l|
    puts l
  end
end
