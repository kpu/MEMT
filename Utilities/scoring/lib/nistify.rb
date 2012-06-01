# Carnegie Mellon University
# Copyright (c) 2009
# All Rights Reserved.
#
# Any use of this software must follow the terms
# outlined in the included LICENSE file.
#

#Makes NIST XML files from scratch.
require 'cgi'
require SCORE_DIR + '/lib/shell_escape'

def expect_both(source, out, line)
  throw "Expected #{line}" unless source.gets.chomp == line
  out.puts line
end

#src is the source file
#text is text to put in
#out is the output file
#sysid is the sysid or refid
#ref is the number of references if ref
def nistify_set(src, text, out, sysid, ref, trglang)
  type = ref ? "ref" : "sys"
  set_tag = (ref ? "ref" : "tst") + "set"

  text_index = 0
  #tstset loop
  loop do
    srcset = src.gets.chomp
    if srcset == "</mteval>" then
      return
    end
    match = srcset.match /^<srcset setid="([^"]*)" srclang="([^"]*)">$/
    throw "Expected a srcset line, got #{srcset}" unless match
    setid = match[1]
    srclang = match[2]
    out.puts "<#{set_tag} setid=\"#{setid}\" srclang=\"#{srclang}\" trglang=\"#{trglang}\" #{type}id=\"#{CGI::escapeHTML(sysid)}\">"
    #doc loop
    loop do
      doc = src.gets.chomp
      if doc == "</srcset>" then
        out.puts "</#{set_tag}>"
        break
      end
      doc_match = doc.match /^<doc docid="([^"]*)" genre="(nw|wb|bo)">$/
      /"/
      # above line just to get vim back to normal
      if doc == "" then
        puts ""
        next
      end
      throw "Bad doc #{doc}" unless doc_match
      docid = doc_match[1]
      genre = doc_match[2]
      out.puts "<doc docid=\"#{docid}\" genre=\"#{genre}\">"
      # seg loop
      loop do
        seg = src.gets.chomp
        if seg == "</doc>" then
          out.puts "</doc>"
          break
        end
        if ["<hl>", "</hl>", "<p>", "</p>"].include?(seg) then
          out.puts seg
          next
        end
        seg_match = seg.match /^<seg id="([0-9]*)">([^<]*)<\/seg>$/
        throw "Bad seg line #{seg}" unless seg_match
        throw "Double segment line #{seg}" if seg_match[1].index("<seg")
        out.write "<seg id=\"" + seg_match[1] + "\">"
        throw "Too few lines in raw file" if text_index == text.size
        seg_data = text[text_index]
        text_index += 1
        seg = CGI::escapeHTML(seg_data.strip)
        out.write(if seg.empty? then
          $stderr.puts "Substituting EMPTYLINE in segment #{seg_match[1]}"
          "EMPTYLINE"
        else
          seg
        end)
        out.puts "</seg>"
      end
    end
  end
  
  last = raw.gets
  throw "Too many lines in raw text file starting with #{last}" if last
end

def nistify_file(src, text, out, sysid, ref, trglang)
  expect_both(src, out, '<?xml version="1.0" encoding="UTF-8"?>')
  expect_both(src, out, '<!DOCTYPE mteval SYSTEM "ftp://jaguar.ncsl.nist.gov/mt/resources/mteval-xml-v1.3.dtd">')
  expect_both(src, out, '<mteval>')

  position = src.tell
  text.each_index do |i|
    src.seek(position)
    reported_sysid = sysid
    reported_sysid += i.to_s if ref
    nistify_set(src, text[i], out, reported_sysid, ref, trglang)
  end

  out.puts '</mteval>'
end

#Split an interleaved array into an array indexed by reference then segment
def separate_interleaved_text(from, refs)
  count = (refs || 1)
  out = []
  count.times { out << [] }
  entry = 0
  from.each do |l|
    l.strip!
    out[entry] << l
    entry = 0 if (entry += 1) == count
  end
  out
end

def nistify_faux_source(text, xml)
  xml.puts '<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE mteval SYSTEM "ftp://jaguar.ncsl.nist.gov/mt/resources/mteval-xml-v1.3.dtd">
<mteval>
<srcset setid="bogus" srclang="any">
<doc docid="bogus" genre="bo">'
  segid = 0
  text.each do |line|
    xml.puts '<seg id="' + (segid += 1).to_s + '">' + CGI::escapeHTML(line.strip) + '</seg>'
  end
  xml.puts '</doc>
</srcset>
</mteval>'
end

def score_nist(request)
  source_name = request.output.temp("faux_source.xml")
  source = File.new(source_name, "w+")
  test_name = request.output.temp("test.xml")
  test = File.new(test_name, "w")
  ref_name = request.output.temp("ref.xml")
  ref = File.new(ref_name, "w")
  nistify_faux_source(request.hyp.lines, source)
  source.rewind
  nistify_file(source, [request.hyp.lines], test, "bogus", nil, request.language)
  source.rewind
  nistify_file(source, separate_interleaved_text(request.ref.laced_lines, request.ref.number), ref, "bogus", request.ref.number, request.language)
  source.close
  test.close
  ref.close
  bleu_out = request.output.perm("bleu_out")
  args = [SCORE_DIR + "/mteval-v13.pl", "-r" ,ref_name, "-s", source_name, "-t", test_name]
  args << "-c" if request.cased
  system_with_redirect(args, nil, bleu_out)
  score_line = File.new(bleu_out).readlines[7]
  throw "Missing score line in #{bleu_out}" unless score_line
  matched = score_line.match(/^NIST score = *([0-9]*.[0-9]*) *BLEU score = *([0-9]*.[0-9]*) for system "bogus"$/)
  throw "Bad score line in #{bleu_out}: #{score_line}" unless matched
  #NIST, IBM
  [matched[1].to_f, matched[2].to_f]
end
