
#!/usr/bin/perl

#** @file vs_plg_to_bin.pl
# @verbatim
#####################################################################
# This program is not guaranteed to work at all, and by using this  #
# program you release the author of any and all liability.          #
#                                                                   #
# You may use this code as long as you are in compliance with the   #
# license (see the LICENSE file) and this notice, disclaimer and    #
# comment box remain intact and unchanged.                          #
#                                                                   #
# Purpose: to convert plugin for VLSI's Vdsp's to binary images     #
# as provided by VLSI's and or VIDE.                                #
#                                                                   #
# example usage: vs_plg_to_bin.pl .\vs1053pcm.plg .\pcm.053         #
#                                                                   #
##################################################################### 
# @endverbatim
#*
use strict;
use warnings;

#** @var $inF
# Input Arguement of Filename to be processed.
#*
my $inF = $ARGV[0] or die "Need input file.\n";
if ($inF !~ m/.plg$/i) {
	print "Input file must be plg extension.\n";
	exit(1);
}

#** @var
# Output Arguement of Filename to be created.
#*
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
