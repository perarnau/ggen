# This is file is free software, protected by the CeCiLL Licence.
# See the LICENCE file for more information
# Copyright Swann Perarnau 2009

# This file contains all the arguments we want to launch ggen with.
# Used by validation_campaign
our @methods = ("erdos_gnp", "erdos_gnm");

## GNP arguments
# For every probability in @probs, generate graphs
# with between 10 and 200 vertices.
my @gnp_args;
my @probs = (0.3, 0.5, 0.7);
for(my $i = 10; $i <= 200; $i+=10) {
	foreach my $p (@probs) {
		push @gnp_args, $i." ".$p;
	}
}

## GNM arguments
# For every probability in probs, generate graphs
# with between 10 and 200 vertices and with
# a number of edges at a constant factor of n
my $gnm_args;
@probs = (0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9);
for(my $i = 10; $i <= 200; $i+=10) {
	for(my $j = 1; $j <= 10; $j++) {
		my $m = $i*$j;
		foreach my $p (@probs) {
			push @gnm_args, $i." ".$m." ".$p;
		}
	}
}

## Global
our %arguments = (
	"erdos_gnp"	=> \@gnp_args,
	"erdos_gnm"	=> \@gnm_args,	
);
1;
