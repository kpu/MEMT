#!/usr/bin/env ruby
require 'cgi'

src=File.new(ARGV[0])
raw=File.new(ARGV[1])
sysid=ARGV[2]
throw "Command line is source_xml raw_txt sysid" unless sysid

out=$stdout

def expect_both(source, out, line)
  throw "Expected #{line}" unless source.gets.chomp == line
  out.puts line
end

expect_both(src, out, '<?xml version="1.0" encoding="UTF-8"?>')
expect_both(src, out, '<!DOCTYPE mteval SYSTEM "ftp://jaguar.ncsl.nist.gov/mt/resources/mteval-xml-v1.3.dtd">')
expect_both(src, out, '<mteval>')

#tstset loop
loop do
  srcset = src.gets.chomp
  if srcset == "</mteval>" then
    out.puts '</mteval>'
    break
  end
  match = srcset.match /^<srcset setid="([^"]*)" srclang="([^"]*)">$/
  throw "Expected a srcset line, got #{srcset}" unless match
  setid = match[1]
  srclang = match[2]
  out.puts "<tstset setid=\"#{setid}\" srclang=\"#{srclang}\" trglang=\"English\" sysid=\"#{CGI::escapeHTML(sysid)}\">"
  #doc loop
  loop do
    doc = src.gets.chomp
    if doc == "</srcset>" then
      out.puts "</tstset>"
      break
    end
    doc_match = doc.match /^<doc docid="[^"]*" genre="(nw|wb)">$/
    /"/
    # above line just to get vim back to normal
    throw "Bad doc #{doc}" unless doc_match
    genre = doc_match[1]
    out.puts doc
    # seg loop
    loop do
      seg = src.gets.chomp
      if seg == "</doc>" then
        out.puts "</doc>"
        break
      end
      if seg == "<hl>" then
        out.puts "<hl>"
        next
      end
      if seg == "</hl>" then
        out.puts "</hl>"
        next
      end
      if seg == "<p>" then
        out.puts "<p>"
        next
      end
      if seg == "</p>" then
        out.puts "</p>"
        next
      end
      seg_match = seg.match /^<seg id="([0-9]*)">(.*)<\/seg>$/
      throw "Bad seg line #{seg}" unless seg_match
      throw "Double segment line #{seg}" if seg_match[1].index("<seg")
      out.write "<seg id=\"" + seg_match[1] + "\">"
      seg_data = raw.gets
      throw "Too few lines in raw file" unless seg_data
      out.write CGI::escapeHTML(seg_data.strip)
      out.puts "</seg>"
    end
  end
end

last = raw.gets
throw "Too many lines in raw text file starting with #{last}" if last
