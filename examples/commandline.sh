#!/bin/sh

# A generation, annotation and analysis example for the command line tool.
# Generate a collection of graphs in ./graphs from GNP, add vertex and edge
# properties, and list a few structural properties of each graph.
# technically you can use the same file over and over, but we want to keep the
# intermediate states for documentation.
# run with ./commandline.sh 2>/dev/null to remove GSL stderr annoying messages

# Configuration:
# your --prefix path (/usr/local/lib by default)
GGEN_INSTALL_PATH=/usr/local

export LD_LIBRARY_PATH=$GGEN_INSTALL_PATH/lib:$LD_LIBRARY_PATH
export PATH=$GGEN_INSTALL_PATH/bin:$PATH

# Graphs generation:
# remove graph directory
rm -rf graphs
mkdir -p graphs
# initialize the random number generator
export GSL_RNG_SEED=`date +%s`
echo "GSL Seed: $GSL_RNG_SEED"
# make sure there isn't a rng state file already
rm -f rng.state

echo "Graph Generation: `date +%s`"
for n in 10 20 30 40 50 100 200 400
do
	for p in 0.05 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1
	do
		# generate a gnp graph with $n nodes and $p probability
		dotfile="graphs/$n.$p.dotfile"
		ggen generate-graph gnp $n $p -l 0 -o $dotfile -r rng.state
		# add a single source to the graph
		sourcefile="graphs/$n.$p.sourcefile"
		ggen transform-graph add-source source -l 0 -i $dotfile -o $sourcefile
	done
done

# Annotate the vertices, with an integer between 1 and 9 (uniform distribution)
echo "Vprop: `date +%s`"
for n in 10 20 30 40 50 100 200 400
do
	for p in 0.05 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1
	do
		sourcefile="graphs/$n.$p.sourcefile"
		vpropfile="graphs/$n.$p.vpropfile"
		ggen add-property uniformint 1 10 --vertex --name weight -l 0 -i $sourcefile -o $vpropfile -r rng.state
	done
done

# Annotate the edges, with an integer between 1 and 20 (uniform distribution)
echo "Eprop: `date +%s`"
for n in 10 20 30 40 50 100 200 400
do
	for p in 0.05 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1
	do
		vpropfile="graphs/$n.$p.vpropfile"
		propfile="graphs/$n.$p.propfile"
		ggen add-property uniformint 1 21 --edge --name size -l 0 -i $vpropfile -o $propfile -r rng.state
	done
done

# List a few structural properties for each dag
echo "Analysis: `date +%s`"
for n in 10 20 30 40 50 100 200 400
do
	for p in 0.05 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1
	do
		propfile="graphs/$n.$p.propfile"
		analysisfile="graphs/$n.$p.analysis"
		echo "Minimum Spanning Tree" >> $analysisfile
		ggen analyse-graph mst -i $propfile -l 0 >> $analysisfile
		echo "Longest Path" >> $analysisfile
		ggen analyse-graph lp -i $propfile -l 0 >> $analysisfile
		echo "Outdegree" >> $analysisfile
		ggen analyse-graph out-degree -i $propfile -l 0 >> $analysisfile
	done
done

echo "Done: `date +%s`"

