#!/usr/bin/env ruby
require 'pathname'
AVENUE_DIR = File.dirname(Pathname.new(File.expand_path(__FILE__) + '/../../..').realpath)

def language_args(corpus)  
  language = File.new(corpus + "/language").read.strip
  case language
    when "en" then "-l en -a /home/kheafiel/meteor-paraphrase/par.en"
    when "cz" then "-l cz -m \"exact paraphrase\" -a /home/kheafiel/meteor-paraphrase/par.cz"
    when "es" then "-l es -m \"exact stem paraphrase\" -a /home/kheafiel/meteor-paraphrase/par.es"
    when "de" then "-l de -m \"exact stem paraphrase\" -a /home/kheafiel/meteor-paraphrase/par.de"
    when "fr" then "-l fr -m \"exact stem paraphrase\" -a /home/kheafiel/meteor-paraphrase/par.fr"
    else throw "Don't know what to do with language #{language}"
  end
end

def match(corpus, name, systems)
  language = language_args(corpus)
  Dir.mkdir(corpus + "/match") unless File.exists?(corpus + "/match")
  out_dir = corpus + "/match/" + name
  Dir.mkdir(out_dir) unless File.exists?(out_dir)
  File.new(out_dir + '/systems', "w").write systems.map { |s| s + "\n" }.join
  Dir.glob(corpus + "/sys/" + systems[0] + "/*").select do |cond|
    cond_name = cond.split('/')[-1]
#next false unless cond_name == "part2"
    systems.each do |s|
      fil = corpus + "/sys/" + s + '/' + cond_name + "/tok"
      unless File.exists?(fil)
        $stderr.puts fil
        break false
      end
    end
  end.map do |cond|
    cond.split('/')[-1]
  end.each do |cond|
    tok = systems.map do |sys|
      corpus + "/sys/" + sys + '/' + cond + "/tok"
    end
    Dir.mkdir(out_dir + '/' + cond) unless File.exists?(out_dir + '/' + cond)
    system(AVENUE_DIR + "/MEMT/Alignment/match.sh " + tok.join(' ') + " #{language} >" + out_dir + '/' + cond + "/matched")
  end
end

corpus=ARGV[0]
name=ARGV[1]
systems=ARGV[2..-1]
throw "corpus name system1 system2 ..." if not systems or systems.empty?
match(corpus, name, systems)
