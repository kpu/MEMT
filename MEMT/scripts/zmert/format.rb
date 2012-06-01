def read_weights(file)
  weights = []
	while (l = file.gets)
    l.strip!
    splt = l.split(' ')
    match = splt[0].match /^param([0-9]+)/
    throw "Bad line #{l}" unless match
		throw "Expected parameters in order, got #{match[1].to_i} instead of #{weights.size}" unless match[1].to_i == weights.size
		weights << splt[1].to_f
	end
  weights
end

def make_config(weights, rest)
  'score.weights = ' + read_weights(File.new(weights)).join(' ') + "\n" + File.new(rest).read
end

class OneBestOutput
  def initialize(io)
    @out = io
  end
  def add(sentence_num, rank, sentence, scores)
    @out.puts sentence if rank == 0
  end
  def finish(sentence_num, count)
    @out.puts "#No output" if count == 0
  end
  def flush
    @out.flush
  end
end

class NBestOutput
  def initialize(io_nbest)
    @out = io_nbest
  end
  def add(sentence_num, rank, sentence, scores)
    @out.write sentence_num.to_s
    @out.write " ||| "
    @out.write sentence
    @out.write " ||| "
    @out.write scores
    @out.write "\n"
  end
  def finish(sentence_num, count)
  end
  def flush
    @out.flush
  end
end

def read_nbest(from, to)
  preindex = nil
  rank = 0
  from.each_line do |l|
    split = l.split(" ||| ")
    throw "Wrong number of splits in #{l}" unless split.size == 3
    index = split[0].to_i
    if (index != preindex)
      to.finish(preindex, rank) if preindex
      preindex = index
      rank = 0
    end
    to.add(index, rank, split[1], split[2])
    rank += 1
  end
  to.finish(preindex, rank) if preindex
end

#Output format used by zmert:
#i ||| translation ||| features
class ZMERTOutput
  attr_reader :name_1best, :name_nbest
  def initialize(name_1best, name_nbest)
    @name_1best = name_1best
    @name_nbest = name_nbest
    @best1 = OneBestOutput.new(File.new(name_1best, "w"))
    @nbest = NBestOutput.new(File.new(name_nbest, "w"))
  end

  def add(sentence_num, rank, sentence, scores)
    @best1.add(sentence_num, rank, sentence, scores)
    @nbest.add(sentence_num, rank, sentence, scores)
  end

  def finish(sentence_num, count)
    @best1.finish(sentence_num, count)
    @nbest.finish(sentence_num, count)
  end

  def flush
    @best1.flush
    @nbest.flush
  end
end
