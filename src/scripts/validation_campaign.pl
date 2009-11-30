#!/usr/bin/perl
# This is file is free software, protected by the CeCiLL Licence.
# See the LICENCE file for more information
# Copyright Swann Perarnau 2009

# This script generate a LOT of graphs for each method implemented
# in GGen, analyse them and create gnuplot graphs.
use strict;
use Getopt::Long;
my $verbose = 0;
my $iterations = 100000;
my $prefix = "validation/";
my $config_file = "config.pl";
use constant GGEN => "@EXECUTABLE_OUTPUT_PATH@/ggen";

sub dbg {
	my $msg = shift;
	print "trace: " . $msg . "\n" if $verbose > 0;
}

#################################################
# ANALYSIS METHODS
#################################################

sub lp {
	dbg("computing longuest path of generated graphs");
	my @files = glob("*.dot");
	for my $f (@files) {
		my $outname, $number;
		($outname = $f) =~ s/(.*)\.\d+\.dot/$1.lp/;
		($number = $f) =~ s/(.*)\.(\d+)\.dot/$2/;
		open(my $out, ">>",$outname) or die "Can't open outfile: $!";

		my @args = ("analyse-graph", "lp", "-i $f");
		open(my $fh,"-|",GGEN,@args) or die "Can't run program: $!";
		while(<$fh>) {
			print $out "$number:" . $_;
		}
		close $fh;
		close $out;
	}
}


#################################################
# MAIN CODE
#################################################

sub analyse {
	dbg("executing the analysis command");
	foreach my $a (@Config::analysis) {
		 eval $a;
	}
}

sub run {
	dbg("executing the run command");
	foreach my $m (@Config::methods) {
		my $ref_args = $Config::arguments{$m};
		my @args = @$ref_args;
		
		foreach my $ar (@args) {
			dbg($m.": args: ".$ar.", generating $iterations graphs");
			my $inlined_args = join("_",split(/\s+/,$ar));

			# init rng file
			my $rng_file = $m."_".$inlined_args."_rng";
			unlink $rng_file;
			dbg($m.": RNG file is ".$rng_file);
			
			for(my $i = 0; $i < $iterations; $i++) {
				my $outfile = $m."_".$inlined_args.".".$i.".dot";
				my $cmd = "generate-graph --output ".$outfile." ".$m." ".$ar;
				my @cmd_args = split(/\s+/,$cmd);
				system(GGEN,@cmd_args) == 0 or die "Something wrong is going on";
			}
		}
	}
}


GetOptions(	'verbose' 	=> \$verbose,
		'iter=i'	=> \$iterations,
		'prefix=s'	=> \$prefix,
		'config=s'	=> \$config_file,
		) or die "Cannot parse options : $!";

{ package Config; do $config_file or die "Config file not found";}

mkdir $prefix;
chdir $prefix or die "Wrong prefix $prefix";

if(scalar(@ARGV) == 0 || $ARGV[0] =~ /run/)
{
	run();
}
elsif($ARGV[0] =~ /analyse/)
{
	analyse();
}
else
{
	die "Wrong command";
}

