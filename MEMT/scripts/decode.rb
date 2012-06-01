#At the top is implementation.  See the public API below.  
require 'socket'
require 'thread'

require AVENUE_DIR + '/MEMT/scripts/match.rb'

def conn_to_socket(conn)
  conn_split = conn.to_s.split(':')
  host = nil
  port = nil
  if conn_split.size == 1 then
    host = "localhost"
    port = conn.to_i
  else
    host = conn_split[0]
    port = conn_split[1].to_i
  end

  TCPSocket.new(host, port)
end

def send_config(server, config)
  server.puts "config"
  server.puts config.size
  server.write(config)
end

def writer_thread(server, config, matched)
  write_count = 0
  begin
    send_config(server, config)
    while p = matched.packet
      server.write(p)
      write_count += 1
    end
    $stderr.puts "Wrote #{write_count}"
    server.puts "bye"
    server.flush
  rescue
    $stderr.puts "Writer encountered #{$!}."
    exit 1
  end
	write_count
end

class OutputQueueInfo
  attr_reader :sentence, :scores 
  def initialize(sentence, scores)
    @sentence = sentence
    @scores = scores
  end
end

def reader_thread(server, queue, detok = nil)
  read_count = 0

  while reply = server.gets do
    throw "Decoder returned error \"#{server.gets.chomp}\"" if reply.strip == "error"
    throw "Unexpected decoder reply \"#{reply.chomp}\"" unless reply.chomp == "nbest"
    count_line = server.gets.chomp
    splt = count_line.split
    throw "Bad split #{count_line}" if splt.size != 2
    throw "Bad sentence id in #{count_line}" if splt[0] != "0"
    count = splt[1].to_i
    queue << count
    count.times do |rank|
      sentence = server.gets.strip
      if detok then
        detok.puts sentence
      end
      scores = server.gets
      queue << OutputQueueInfo.new(sentence.strip, scores.strip)
    end
    read_count += 1
    $stderr.puts "Read #{read_count}" if read_count % 5 == 0
  end
  queue << nil
  if detok then
    detok.flush
    detok.close_write
  end
	read_count
end

def detokenizer_program(language)
  case language
    when "cz" then AVENUE_DIR + "/Utilities/Tokenization/Moses/detokenizer.perl -l cs"
    else AVENUE_DIR + "/Utilities/Tokenization/WMT/detokenizer.perl -l #{language}"
  end
end

def write_output_thread(queue, output, detok = nil)
  read_count = 0
  while count = queue.pop
    count.times do |rank|
      info = queue.pop
      sentence = if detok then detok.gets.strip else info.sentence end
      output.add(read_count, rank, sentence, info.scores)
    end
    output.finish(read_count, count)
    read_count += 1
  end
  output.flush
  read_count
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

def raw_feature_info(conn, config, num_systems)
  server = conn_to_socket(conn)
  send_config(server, config + "\nnum_systems = #{num_systems}\nscore.weights = uniform\n")
  server.puts "feature_info"
  reply = server.gets
  throw "Bad feature count #{reply}" if reply.to_i <= 0
  ret = server.gets.split(' ').map { |i| i.to_i }
  server.puts "bye"
  server.close
  ret
end

def custom_decode(config, conn, matched, output, postprocess)
  start = Time.now

  config = config + "\nnum_systems = #{matched.sys_count}\n"
  $stderr.puts "Running decoder on\n#{config}"

  server = conn_to_socket(conn)

	write_count = nil
  writer = thread_die_on_exception { write_count = writer_thread(server, config, matched) }
  read_count = nil
  queue = Queue.new
	reader = thread_die_on_exception { read_count = reader_thread(server, queue, postprocess) }
  write_output_count = write_output_thread(queue, output, postprocess)
  writer.join
  reader.join

  throw "Only read #{read_count} sentences while #{write_count} were sent." unless read_count == write_count
  throw "Only read #{write_output_count} sentences while #{read_count} were sent." unless write_output_count == read_count
  server.close
	output.flush
  postprocess.close if postprocess

  took = Time.now - start
  $stderr.puts "Decoding #{write_count} sentences took #{took} seconds (#{write_count.to_f / took} sentences/second)"
  read_count
end

def get_detok_language(language)
#disable detokenizer
  return nil
  return nil if language == nil
  splt = language.split('_')
  language = (splt.size == 1) ? splt[0] : splt[1]
  return nil if language == "none"
  return language
end

def raw_decode(config, conn, matched, output, language = nil)
  detok_language = get_detok_language(language)
  detok = if detok_language then
    IO.popen(detokenizer_program(detok_language), "r+")
  end
  custom_decode(config, conn, JavaMatched.new(matched), output, detok)
end

def identify_microsoft(config, need_weights)
  weights = if need_weights
    matched = config.match(/^\s*score.weights\s*=\s*(.*)$/)
    throw "Could not find weights in #{config}" unless matched
    matched[1].split(' ')
  end

  if config.match(/^\s*microsoft/)
    raw_config = config.sub(/^\s*microsoft/, "")
    if weights and weights != ["uniform"] then
      raw_config.sub!(/^\s*score.weights\s*=\s*(.*)$/, "score.weights = " + weights[0..-2].join(' '))
    end
    return [true, raw_config, weights]
  else
    return [false, config, weights]
  end
end

def feature_info(conn, config, num_systems)
  msft = identify_microsoft(config, false)
  ret = raw_feature_info(conn, msft[1], num_systems)
  ret += [1] if msft[0]
  ret
end

def decode(config, conn, matched, output, language)
  msft = identify_microsoft(config, true)
  if msft[0] then
    out_pipe = IO.popen(AVENUE_DIR + "/lm/dist/microsoft_nbest /home/kheafiel/cache cluster03:3000 |" + AVENUE_DIR + "/lm/microsoft/rerank.py " + msft[2].join(' ') + ">" + output + ".nbest", "w")
    raw_decode(msft[1], conn, matched, NBestOutput.new(out_pipe), language)
    out_pipe.close
    one_file = File.new(output + ".1best", "w")
    read_nbest(File.new(output + ".nbest"), OneBestOutput.new(one_file))
    one_file.close
  else
    raw_decode(config, conn, matched, ZMERTOutput.new(output + ".1best", output + ".nbest"), language)
  end
end
