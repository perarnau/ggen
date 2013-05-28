# GGen : a graph generator #

GGen is a DAG (Directed Acyclic Graph) generation library and command line client.

This tool was designed to help Operation Research (Scheduling)
researchers with the simulation of their algorithms. It provides a
collection of classical workload generation methods, algorithms to
annotate and analyse the generated graph.

GGen use internally the [Igraph][igraph] and [GNU Scientific Library][gsl]
libraries for its data structures and random number generators.

The command line client uses [DOT][] as its textual graph file format.

Address any remarks concerning compiling or using this tool to
[Swann Perarnau (main developer)][mail].

## REQUIREMENTS ##

### Compiling from distribution package ###

While a stable release (0.3) was made a long time ago, it is not supported
anymore. You can still try to build it, and there is a patch available
on demand to make it work, but it is NOT recommended.

Please refer to its README for instructions.

### Compiling from repository ###

- autotools (libtoolize, autoconf, automake, autoheaders, aclocal)
- Igraph (>= 0.6)
- GNU Scientific Library 
- Cgraph (graphviz) 
- pkg-config (optional: you can tell configure were to find the other libs manually)
- pandoc

./autogen.sh && ./configure && make && make install

## Documentation ##

Man pages are generated automatically by make. The GGen website will eventually contain more
information.

The command line client as an extensive --help. It displays help recursively on modules, commands
and required arguments.

## Additional Info ##

If you still have questions, mail us.

[igraph]: http://igraph.sourceforge.net
[gsl]: http://www.gnu.org/software/gsl/
[DOT]: http://www.graphviz.org
[mail]: mailto:swann.perarnau@imag.fr
[web]: http://ggen.ligforge.imag.fr
