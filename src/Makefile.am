AM_CPPFLAGS = @CGRAPH_CFLAGS@ @IGRAPH_CFLAGS@ @GSL_CFLAGS@

lib_LTLIBRARIES = libggen.la

LIBCSOURCES = graph-generation.c graph-analysis.c graph-transformation.c vector_utils.c vector_utils.h bipartite-matching.c bipartite-matching.h tree-lowest-common-ancestor.c tree-lowest-common-ancestor.h

LIBHSOURCES = ggen.h

libggen_la_SOURCES = $(LIBCSOURCES) $(LIBHSOURCES)
include_HEADERS = $(LIBHSOURCES)

bin_PROGRAMS = ggen
ggen_SOURCES = builtin-generate-graph.c ggen-tool.c builtin-analyse-graph.c \
	       builtin-transform-graph.c builtin-add-property.c \
	       builtin-analyse-property.c builtin.h utils.h utils.c log.c log.h
ggen_LDADD = libggen.la @CGRAPH_LIBS@ @IGRAPH_LIBS@ @GSL_LIBS@
