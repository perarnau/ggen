# automake config
AM_COLOR_TESTS = yes

# flags for building check programs
AM_CFLAGS = -I$(top_srcdir)/src @GSL_CFLAGS@ @IGRAPH_CFLAGS@
LDADD = ../src/libggen.la @GSL_LIBS@ @IGRAPH_LIBS@

if TEST_VALGRIND
TESTS_ENVIRONMENT= @LIBTOOL@ --mode=execute @VALGRIND@ --tool=memcheck -q --leak-check=full
endif

# all check programs
TST_PROGS = erdos_gnp erdos_gnm erdos_lbl fifo random_orders longest_path \
	    transform_add transform_delete errors

check_PROGRAMS = $(TST_PROGS)
TESTS = $(TST_PROGS)
