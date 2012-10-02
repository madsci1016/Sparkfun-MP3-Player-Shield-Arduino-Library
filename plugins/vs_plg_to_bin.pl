#!/usr/bin/perl
#
# Converts VSLI's VS1xxx VSIDE plugin's to raw binary file
#
# usage  : vs_plg_to_bin.pl input.plg [output.vs]
# where input.plg is provided by VSLI's down load software
#  http://www.vlsi.fi/en/support/software.html
# [output.vs] is optional, default is to output input.plg
#
# example: vs_plg_to_bin.pl c:\rtmidistart.plg
# output: c:\rtmidistart.vs
#

use strict;
use warnings;

my $inF = $ARGV[0] or die "Need input file.\n";
if ($inF !~ m/.plg$/i) {
	print "Input file must be plg extension.\n";
	exit(1);
}

my $outF = $ARGV[1] || $inF; # create the name of the output file name, if not provided.
$outF =~ s/.plg$/.vs/i;

open(my $infile, '<', $inF) or die "Could not open '$inF' $!\n";

while (my $line = <$infile>) # read each line
{
	chomp $line;
	if ($line =~ m/short\splugin\[/i) # looking for begin of actual data.
	{
		open(my $outfile, '>:raw', $outF) or die "Unable to open: $!";
		# In the above line ':raw' in the call to open tells it to put
		# the filehandle into binary mode on platforms where that matters
		# (it is equivalent to using binmode).

		while (my $line = <$infile>) # read each of the remaining line
		{
			while ($line =~ m/0x([0-9A-F]{1,4})/gi) # global matching for other instances on same line.
			{
				print "0x" . $1 . " ";
				print $outfile pack('s<', hex($1));
				# In the above line, hex converts the string of hex found to a scalar integer
				# The pack formats the scalar, the 's' tells it to output a signed short (16 bits),
				# and the '>' forces it to big-endian mode, and '<' is little-endian.
			}
			print "\n";
		}
		close($outfile);
	}
}
close($infile);
