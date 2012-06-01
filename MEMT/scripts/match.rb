class JavaMatched
  attr_reader :sys_count
  def initialize(file)
    @file = file
    @file.seek(0)
    @sys_count = @file.gets.to_i
    throw "Bad count" unless @sys_count > 0
    @file.seek(0)
  end
  def get_match
    count = @file.gets
    return nil unless count
    ret = count
  
    count.to_i.times do
      ret += @file.gets
    end
    ((count.to_i * (count.to_i - 1))/2).times do
      while true do
        line = @file.gets
        ret += line
        break if line == "\n"
      end
    end
  
    ret
  end

  def packet
    str = get_match
    return nil unless str
    "matched 0\njava\n" + str
  end
end
