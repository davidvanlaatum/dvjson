#!/usr/bin/env perl

use strict;
use Cwd 'abs_path';

open(IN, "<$ARGV[0]") || die($!);
my %PATHS;

while (<IN>) {
    while (/(-isystem|-I) ?(\S+)/g) {
        $PATHS{abs_path($2)} = 1;
    }
}

foreach my $key (keys %PATHS) {
    if ($key !~ m|^$ARGV[1]|) {
        print "$key\n";
    }
}
