# GGen : a graph generator #


GGen is a DAG (Directed Acyclic Graph) generation library and command line client.

This tool was designed to help scheduling researchers with the simulation of their
algorithms. It provides a collection of classical workflow generation methods,
algorithms to annotate and analyse the generated graph.

GGen use internally the [Igraph][] and [GNU Scientific Library][] libraries for its data structures
and random number generators.

The command line client uses [DOT][] as its textual graph file format.

Address any remarks concerning compiling or using this tool to the bug tracker
of the [GGen Website] or the [GGen Mailing-list][].

## REQUIREMENTS ##

### Compiling from distribution package ###

- make
- Igraph
- GNU Scientific Library 
- Cgraph (graphviz) 
- pandoc (for man pages only)
- pkg-config (optional: you can tell configure were to find the other libs manually)

./configure && make && make install

### Compiling from repository ###

- autotools (libtoolize, autoconf, automake, autoheaders, aclocal)
- Igraph
- GNU Scientific Library 
- Cgraph (graphviz) 
- pkg-config (optional: you can tell configure were to find the other libs manually)

./autogen.sh && ./configure && make && make install

## Documentation ##

Man pages are generated automatically by make. The GGen website will eventually contain more
information.

The command line client as an extensive --help. It displays help recursively on modules, commands
and required arguments.

## Additional Info ##

As usual the INSTALL file contains additional informations.

If you still have questions, mail us.
