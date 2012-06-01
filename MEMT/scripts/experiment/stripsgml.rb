#!/usr/bin/env ruby
require 'cgi'
open = false
$stdin.read.scan /<seg id="?[^>]*"?>(.*)<\/seg>/ do |m|
  $stdout.puts CGI::unescapeHTML(m[0]).gsub(/&apos;/, "'").gsub(/& AMP;/, '&').gsub(/\n/, ' ').strip
end

