# Based on Escape class from http://www.a-k-r.org/escape/ by Tanaka Akira.
#Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
#   1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
#   2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
#   3. The name of the author may not be used to endorse or promote products derived from this software without specific prior written permission.
#
#THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#(The modified BSD licence)

def escape_shell_word(str)
  if str.empty?
    "''"
  elsif %r{\A[0-9A-Za-z+,./:=@_-]+\z} =~ str
    str
  else
    result = ''
    str.scan(/('+)|[^']+/) {
      if $1
        result << %q{\'} * $1.length
      else
        result << "'#{$&}'"
      end
    }
    result
  end
end

def escape_shell_array(arr)
  arr.map do |str| escape_shell_word(str) end.join(' ')
end

def system_with_redirect(command_arr, from, to)
  command = escape_shell_array(command_arr)
  command += " <" + escape_shell_word(from) if from
  command += " >" + escape_shell_word(to) if to
  throw "Command failed: #{command}" unless system(command)
end

def append_colon_environment(name, value)
  value = escape_shell_word(value)
  if ENV[name]
    ENV[name] += ":" + value
  else
    ENV[name] = value
  end
end
