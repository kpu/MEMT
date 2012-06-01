#!/usr/bin/env ruby

class Scores
  attr_reader :arr
  def initialize(file)
    @arr = File.new(file).read.split(" ").map { |s| s.to_f }
  end
  def to_s
    formats = ["%5.2f", "%5.2f", "%5.2f", "%5.2f", "%5.2f", "%5.2f", "%0.02f"]
    ret = []
    @arr.each_index do |i|
      ret << sprintf(formats[i], @arr[i])
    end
    ret.join(' ')
  end
  def sort_key
#   @arr[2] -@arr[0]
   -@arr[0]
  end
  def <=>(other)
    return -1 if other.instance_of?(NilScores)
    sort_key.<=>(other.sort_key)
  end
end

class NilScores
  def to_s
    "                                        "
  end
  def <=>(other)
    return 0 if other.instance_of?(NilScores)
    return 1
  end
end

class PurposeScores
  attr_reader :label, :purposes
  def initialize(files, label)
     @label = label
     @purposes = files.map do |f|
       if File.exists?(f) then
         Scores.new(f)
       else
         NilScores.new
       end
     end
  end

  def <=>(other)
    purposes.size.times do |rev|
      i = purposes.size - rev - 1
#i = rev
      ret = purposes[i].<=>(other.purposes[i])
      return ret if ret != 0
    end
    return label.<=>(other.label)
  end

  def to_s
    @purposes.join(' ') + ' ' + @label
  end
end

def print_scores(scores)
  scores.sort!
  puts "BLEU  NIST  TER   METEO Preci Recal Leng BLEU  NIST  TER   METEO Preci Recal Leng"
  scores.each do |s|
    puts s
  end
end

def status(corpus)
  puts corpus
  scores = []
  Dir.glob(corpus + "/sys/*").each do |d|
    scores << PurposeScores.new([d + "/tune/tok.scores"], d.split('/')[4])
#    scores << PurposeScores.new([d + "/tune/txt.scores"], d.split('/')[4])
  end
  Dir.glob(corpus + "/match/*/runs/*").each do |s|
    scores << PurposeScores.new([s + "/output.1best.scores"], s.split('/')[6])
#    scores << PurposeScores.new([s + "/output.1best.scores"], s.split('/')[6])
  end
  print_scores(scores)
end

DIRS=if ARGV.empty? then
  ["wmt10/cz-en", "wmt10/de-en", "wmt10/es-en", "wmt10/fr-en", "wmt10/xx-en", "wmt10/en-cz", "wmt10/en-de", "wmt10/en-es", "wmt10/en-fr", "wmt09/cz", "wmt09/de", "wmt09/es", "wmt09/fr", "wmt09/hu", "mt09/ar", "mt09/ur", "gale/nw", "gale/wb", "gale/bc", "gale/bn", "gale/audio"]
else
  ARGV.map do |l|
    splt = l.split('/', 2)
    if splt[0] == "corpus"
      splt[1]
    else
      l
    end
  end
end.each do |l|
  status("corpus/#{l}")
end
