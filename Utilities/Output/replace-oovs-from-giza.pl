#!/usr/local/bin/perl

use strict;
use bytes;


# Check usage:
if($#ARGV != 0)
{
	print STDERR "Usage: cat <nbest-output-file> | perl $0 <ti-f2e>\n";
	print STDERR "<ti-f2e> has lines \"e f p(e|f)\".\n";
	print STDERR "Output goes to standard out.\n";
	exit;
}


# Load GIZA lexical probabilities; save the most likely En word for each Fr:
my %FrDict = ();
my %FrProb = ();
open(my $NFFILE, $ARGV[0]) or die "Can't open file $ARGV[0]: $!";
print STDERR "Loading P(e|f) file $ARGV[0]... ";
while(my $line = <$NFFILE>)
{
	my ($en, $fr, $prob) = split(/\s+/, $line);
	if($prob > $FrProb{$fr})
	{
		$FrDict{$fr} = $en;
		$FrProb{$fr} = $prob;
	}
}
print STDERR "done.\n";


# Read through the n-best list file, replacing OOVs as possible.  Since
# full-sentence text comes way before an OOV in it is listed, we have to
# save up lines to fix and print:
my @PrintBuffer = ();
my $outputString = "";
my $outputBufPos = -1;
while(my $line = <STDIN>)
{
	# Save the plaintext output string we currently have:
	if($line =~ /^([0-9]+ [0-9]+\t)(.*)$/)
	{
		# Clear current buffer (previous sentence):
		chomp $PrintBuffer[$outputBufPos];
		$PrintBuffer[$outputBufPos] .= "$outputString\n";
		foreach my $l (@PrintBuffer) { print "$l"; }
		@PrintBuffer = ();
		$outputBufPos = -1;

		# Load again with current line:
		push(@PrintBuffer, $1);
		$outputString = $2;
		$outputBufPos = $#PrintBuffer;
	}

	# Extract information if there's an unknown word and fix it:
    # Line format: ( 27 28 "njrH" -259.439 "njrH" "(UNK,2 'njrH')")
	elsif($line =~ /\(UNK,[0-9]+ \'(.*)\'\)/)
	{
		my $unkWord = $1;

		# Replace the UNK with a GIZA entry if possible:
		if(exists($FrDict{$unkWord}))
		{
			$outputString =~ s/$unkWord/$FrDict{$unkWord}/;
			#print "Change [$unkWord] to [$FrDict{$unkWord}]\n";
			$line =~ s/UNK,/UNKFIX,/;
			#print "$unkWord\t$FrDict{$unkWord}\t$FrProb{$unkWord}\n";
		}

		# Add this line to the print buffer:
		push(@PrintBuffer, $line);
	}

	else
	{
		# Otherwise just save the line for printing:
		push(@PrintBuffer, $line);
	}
}

# Clear final print buffer:
chomp $PrintBuffer[$outputBufPos];
$PrintBuffer[$outputBufPos] .= "$outputString\n";
foreach my $l (@PrintBuffer) { print "$l"; }
