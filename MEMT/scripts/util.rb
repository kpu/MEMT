require 'pathname'
SCRIPT_DIR = File.dirname(Pathname.new(File.expand_path(__FILE__)).realpath)

class Message
  attr_accessor :prefix
	def initialize(prefix, file)
		@prefix = prefix
	  @last = Time.now
	  @file = file
	end
	def measure
		tick = Time.now
		ret = tick - @last
		@last = tick
	  ret
	end
  def tell(func, event)
		message = "#{@prefix}#{event} #{func} at #{Time.now} (#{measure}s)"
		$stderr.puts message
		@file.puts message
		@file.sync
	end

	def wrap(func)
	  tell(func, :start)
	  ret = yield
	  tell(func, :finish)
	  ret
	end
end

def write_close(name, content)
  f = File.new(name, 'w')
  f.write(content)
  f.close
end

#not the most efficient, but it works for references.
def count_lines(file)
	count = 0
	file.each_line do |l|
	  count += 1
	end
	count
end
