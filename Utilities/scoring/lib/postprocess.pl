#!/usr/bin/env perl

# Carnegie Mellon University
# Copyright (c) 2009
# All Rights Reserved.
#
# Any use of this software must follow the terms
# outlined in the included LICENSE file.
# 
# Written by Greg Hanneman

use bytes;
use strict;


# Check for correct usage:
if($#ARGV != -1)
{
    print "Usage: perl $0 <in.txt >out.txt\n";
    exit;
}

# Open input and process lines:
my $punctRE = '(\.|,|:|;|!|\?|\"|\'|\(|\)|\[|\])';
my $nonRE = '[^.,:;!\?\"\'()\[\] ]';
while(my $line = <STDIN>)
{
  chomp $line;

  # Fix HTML characters:
  $line =~ s/& quot ;/\"/g;
  $line =~ s/& amp ;/&/g;
  $line =~ s/& lt ;/</g;
  $line =~ s/& gt ;/>/g;

  # Fix parser things:
  $line =~ s/-LRB-/\(/g;
  $line =~ s/-RRB-/\)/g;
  $line =~ s/\\\//\//g;
  $line =~ s/\\\*/\*/g;

  # Capitalization rules:
  $line =~ s/(^| )i( |$)/\1I\2/g;

  # Rejoin punctuation:
  $line =~ s/ ((n\'t)|(\'s)|(\'m)|(\'re)|(\'ve)|(\'ll)|(\'d)) /\1 /gi;
  $line =~ s/(n?) \' (t|s|m|(re)|(ve)|(ll)|d)\b/\1\'\2/g;
  $line =~ s/ (\.|,|:|;|!|\?|&|\%|\)|\]|>|\})( |$)/\1 /g;
  $line =~ s/ (\d+): (\d+) / \1:\2 /g;
  $line =~ s/ (\d+)([\.]) (\d+) / \1\2\3 /g;
  $line =~ s/( |^)(\$|\(|\[|<|\{) /\1\2/g;
  $line =~ s/( |^)(\\|\/)( |$)/\2/g;

  # Hyphens -- Rejoin if not surrounded by punctuation:
  s/($nonRE) - ($nonRE)/\1-\2/g;
  s/($nonRE$punctRE) - ($nonRE)/\2-\3/g;

  # LaTeX-style quotes:
  $line =~ s/( |^)\`\` / \"/g;
  $line =~ s/ \'\'( |$)/\" /g;

#  \'|\`|\"

  $line =~ s/\s+/ /g;
  $line =~ s/^\s+//;
  $line =~ s/\s+$//;

  print $line . "\n";
}
