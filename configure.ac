# fetch version number
m4_include([version.m4])
# init autoconf
AC_INIT([ggen],[VERSION_NUMBER],[ggen-devel@ligforge.imag.fr])
# are we in the right source dir ?
AC_CONFIG_SRCDIR([src/ggen.h])
# build tools in separate dir
AC_CONFIG_AUX_DIR([build-aux])
# macro in srcdir/m4
AC_CONFIG_MACRO_DIR([m4])

# automake should fail on any error
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

# check for libtool
LT_INIT([])

# check for programs
AC_LANG([C])
AC_PROG_CC
AC_PROG_CC_C_O
AM_PROG_CC_C_O
AC_PROG_CC_STDC
AC_PROG_CPP
AC_TYPE_SIZE_T


# support for testing with valgrind
AC_ARG_ENABLE(valgrind,
[AS_HELP_STRING([--enable-valgrind],[Also valgrind on checks (default is no).])],
[valgrind=true],[valgrind=false])

if [[ "x$valgrind" = xtrue ]]; then
	AC_PATH_PROG(VALGRIND, valgrind, no)
	if [[  "x$VALGRIND" = xno ]]; then
		AC_MSG_ERROR([Valgrind not found in PATH. ])
	fi
fi
AM_CONDITIONAL([TEST_VALGRIND],[test "x$valgrind" = xtrue])

AC_PATH_PROG(PANDOC,pandoc,no)
if [[ "x$PANDOC" = xno ]]; then
	AC_MSG_WARN([Pandoc not found in PATH - will not generate documentation ])
fi
AM_CONDITIONAL([HAVE_PANDOC],[test ! "x$PANDOC" = xno])


#check for GSL
PKG_CHECK_MODULES([GSL],[gsl])
#check for igraph
PKG_CHECK_MODULES([IGRAPH],[igraph])
#check for cgraph
PKG_CHECK_MODULES([CGRAPH],[libcgraph])
#check for libyaml
PKG_CHECK_MODULES([YAML],[yaml-0.1])

# configuration output in config.h
AC_CONFIG_HEADERS([config.h])
# output makefiles
AC_CONFIG_FILES([Makefile src/Makefile tests/Makefile ggen.pc])
# doc
AM_COND_IF([HAVE_PANDOC],[AC_CONFIG_FILES([doc/Makefile])])

# do the output
AC_OUTPUT
