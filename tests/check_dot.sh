#!/bin/sh
set -e
set -u
file=`mktemp`
# output a simple graph
cat >> $file << 'END'
digraph g {
	graph [ color = "red" ];
	0 [ weight = "1" ];
	1 [ weight = "1" ];
	2 [ weight = "3" ];
	0 -> 1 [ size = "2" ];
	1 -> 2 [ size = "1" ];
}
END
out=`mktemp`
# use gvpr to reprint the graph
# this is needed because graphviz
# has some strange behavior on
# string attributes
gvpr -c '' $file > $out
mv $out $file
# compare input and output of our
# graph io functions
./dot_io < $file > $out
diff -q $file $out
# cleanup
rm $file $out
