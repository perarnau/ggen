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

- autotools (libtoolize, autoconf, automake, autoheaders, aclocal)
- Igraph (>= 0.7, might not be packaged for your linux distribution yet)
- GNU Scientific Library 
- Cgraph (graphviz) 
- pkg-config (optional: you can tell configure were to find the other libs manually)
- pandoc

### Compiling from distribution package ###

`./configure && make && make install` should work.

### Compiling from repository ###

`./autogen.sh && ./configure && make && make install`

Note: in order to compile on OS X and some Linux platforms, you may have to
change `libtoolize` in `autogen.sh` to `glibtoolize`.

## Vagrant Setup

This repository contains a Vagrantfile you can use to deploy automatically a
Virtualbox VM with ggen installed correctly.

## Documentation ##

Man pages are generated automatically by make.

The command line client as an extensive --help. It displays help recursively on
modules, commands and required arguments.

## Additional Info ##

If you still have questions, mail us.

[igraph]: http://igraph.sourceforge.net
[gsl]: http://www.gnu.org/software/gsl/
[DOT]: http://www.graphviz.org
[mail]: mailto:swann.perarnau@imag.fr
