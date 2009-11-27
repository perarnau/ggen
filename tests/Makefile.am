LIBTESTS_CPPFLAGS = -I$(srcdir)/../lib
LDADD = $(srcdir)/../lib/libggen.la
AM_COLOR_TESTS = yes

check_PROGRAMS = lib_results
lib_results_SOURCES = lib_results.cpp
lib_results_CPPFLAGS = $(LIBTESTS_CPPFLAGS)

TESTS = $(check_PROGRAMS)
