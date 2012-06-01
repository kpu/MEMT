#!/usr/bin/env ruby
require 'cgi'

$stdin.each_line do |l|
  $stdout.write CGI::unescapeHTML(l).gsub(/&apos;/, "'").gsub(/& AMP;/, '&')
end
