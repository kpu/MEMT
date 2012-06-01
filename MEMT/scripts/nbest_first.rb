#!/usr/bin/env ruby
pre = nil
while l = gets
  splt = l.split("|||")
  num = splt[0].to_i
  if num != pre
    pre = num
    puts splt[1].strip
  end
end
