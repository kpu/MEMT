# Carnegie Mellon University
# Copyright (c) 2010
# All Rights Reserved.
#
# Any use of this software must follow the terms
# outlined in the included LICENSE file.
#
require SCORE_DIR + '/lib/nistify'
require SCORE_DIR + '/lib/terrify'
require SCORE_DIR + '/lib/meteorify'
require SCORE_DIR + '/lib/length'

def flatten_once(arr)
  ret = []
  arr.each do |e|
    ret += e
  end
  ret
end

#Ruby supports making thread exceptions abort.  But it returns 0.   
def die_on_exception
  begin
    yield
  rescue => error
    $stderr.puts error.backtrace
    $stderr.puts error
    exit 1
  end
end

def thread_die_on_exception
  Thread.new do die_on_exception do yield end end
end

class MetaMetric
  def initialize
    @metrics = [NIST, TER, METEOR, Length].map do |m|
      m.new
    end
  end
  def summary_score(request)
    flatten_once(@metrics.map do |m|
      thread_die_on_exception do
        m.summary_score(request)
      end
    end.map do |t|
      t.value
    end)
  end
  def summary_format(request)
    flatten_once(@metrics.map do |m| m.summary_format(request) end)
  end
  def individual_score(request)
    tmp = @metrics.map do |m| s = m.individual_score(request) end
    flatten_once(tmp)
  end
  def individual_format(request)
    flatten_once(@metrics.map do |m| m.individual_format(request) end)
  end
end
