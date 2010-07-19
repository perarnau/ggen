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
AM_INIT_AUTOMAKE([-Wall -Werror gnu])
# check for programs
AC_LANG_C
AC_PROG_CC
AC_PROG_CC_C_O
AM_PROG_CC_C_O
AC_PROG_CC_STDC
AC_PROG_CPP
AC_PROG_LIBTOOL



#check for GSL
PKG_CHECK_MODULES([GSL],[gsl])
#check for igraph
PKG_CHECK_MODULES([IGRAPH],[igraph])
#check for cgraph
PKG_CHECK_MODULES([CGRAPH],[libcgraph])

# configuration output in config.h
AC_CONFIG_HEADERS([config.h])
# output makefiles
AC_CONFIG_FILES([Makefile src/Makefile tests/Makefile])
# do the output
AC_OUTPUT
