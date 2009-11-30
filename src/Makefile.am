AM_CXXFLAGS = $(BOOST_CPPFLAGS) -I$(srcdir)/../lib
AM_LDFLAGS = $(BOOST_LDFLAGS) -l$(BOOST_GRAPH_LIB)

bin_PROGRAMS = ggen
ggen_SOURCES = builtin-generate-graph.cpp ggen.cpp builtin-analyse-graph.cpp builtin-transform-graph.cpp builtin-add-property.cpp builtin-extract-property.cpp usage.cpp
ggen_LDADD = ../lib/libggen.la
