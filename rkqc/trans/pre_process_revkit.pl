#!/usr/bin/perl
use warnings;

use File::Basename;

# Only argument is input scaffold file
# Output filenames are taken from that
$basename = basename( $ARGV[0], qw(.scaffold) );
$scafOut = "$basename\_norevkit.scaffold";
#$scafOut =~ s#.*/(.*)\.scaffold$#$1\_norevkit.scaffold#g;
open FIN, "<", $ARGV[0] or die "Couldn't open source file $ARGV[0]: $!\n";
open SCAF, ">", $scafOut or die "Unable to open scaffold output: $!\n";

$line=0;
@buffer=<FIN>;
$last_name='';
@revkit=();

# Duplicate all global lines between Scaffold and Revkit output
# Any Revkit module is changed to an 'extern void' prototype in Scaffold
# Any Scaffold module is not added to the Revkit output
# All global variables need to be #defines for Revkit
while ($#buffer >= $line) {
    print "$line: $buffer[$line]";

    # Extract Revkit
	if ( $buffer[$line] =~ /^revkit\s+(\w+).*{/ ) {
        $module = $1;
        #print "revkit: $line: $buffer[$line]\n";
        #print Revkit $buffer[$line];
        $buffer[$line] =~ s/revkit\s+(\w+)/void $1/;
        $last_name = $1;
        push @revkit, $buffer[$line];
        $buffer[$line] =~ s/void/extern void/;
        $buffer[$line] =~ s/qint\s*\[?(\s*\d*\s*)\]?\s+(\w+)/qbit $2/g;
        $buffer[$line] =~ s/{/;/;
		print SCAF $buffer[$line];
		$braces = 1;
		while ($braces > 0 and $#buffer >= $line) {
			$line++;
            #print "revkit: $line, $braces\n";
            #print Revkit $buffer[$line];
            push @revkit, $buffer[$line];
			$braces++ if ( $buffer[$line] =~ /{/ );
			$braces-- if ( $buffer[$line] =~ /}/ );
		}
        if ( $line > $#buffer ) {
            die "Unable to find end of Revkit module '$module'\n";
        }

    # Preserve all modules and other functions in Scaffold
    } elsif ( $buffer[$line] =~ /^\w+\s+(\w+).*{/ ) {
        $module = $1;
        #print "module: $line: $buffer[$line]\n";
		print SCAF $buffer[$line];
		$braces = 1;
		while ($braces > 0 and $#buffer >= $line) {
			$line++;
            #print "module: $line, $braces: \n";
			print SCAF $buffer[$line];
			$braces++ if ( $buffer[$line] =~ /{/ );
			$braces-- if ( $buffer[$line] =~ /}/ );
		}
        if ( $line > $#buffer ) {
            die "Unable to find end of Revkit module '$module'\n";
        }

    # Duplicate all compiler directives for scaffold, but only #define for revkit
    } elsif ( $buffer[$line] =~ /^#/ ) {
		print SCAF $buffer[$line];
        push @revkit, $buffer[$line]	if ( $buffer[$line] =~ /^#define/ );

    # Clone all global lines
	} else {
        #print "$line\n";
		print SCAF $buffer[$line];
        #print Revkit $buffer[$line];
        #push @revkit, $buffer[$line];
    }
    $line++;
}

close FIN;
close SCAF;

# Generate Revkit for translation
# Name file with the module to be replaced in the Scaffold
$revkitOut = $ARGV[0];
$revkitOut = "$basename\.$last_name\.cpp";
#$revkitOut =~ s#.*/(.*)\.scaffold$#$1\.$last_name\.revkit#g;
open Revkit, ">", $revkitOut or die "Unable to open revkit output: $!\n";
print Revkit "#include <iostream> \n";
print Revkit "#include <core/circuit.hpp> \n";
print Revkit "#include <core/functions/add_gates.hpp> \n";
print Revkit "#include <boost/lexical_cast.hpp> \n";
print Revkit "using namespace revkit; \n";
foreach $line (@revkit) {
    # Replace the last module with 'main_module' for revkit
    if ( $line =~ s/void\s*$last_name(\w*)/int main (int argc, char** argv){\n/ ){
        $lastline= "int main (int argc, char** argv){\n"; 
        print Revkit "// $last_name = main_module\n"; 
        print Revkit $lastline; 
    }
    else{
        print Revkit $line;
    }
};
close Revkit;

