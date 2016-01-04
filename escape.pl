#!/usr/bin/perl

# For use in including source code into HTML files
# replaces entities such as '<' with the appropriate escapes '&lt;'

use HTML::Entities;

chdir($ARGV[0]) or die "can't chdir to $ARGV[0]";

local $/=undef;
open(FILE, $ARGV[1]) or die "no such file $ARGV[1]";
binmode FILE;
$program = <FILE>;
close FILE;

# print "<pre class=\"sh_cpp\">";
my $escaped = encode_entities($program, "<>&");

print $escaped;
# print "\n</pre>";

exit 0;










