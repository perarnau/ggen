# fetch version number
m4_include([version.m4])
# init autoconf
AC_INIT([ggen],[VERSION_NUMBER],[ggen-devel@ligforge.imag.fr])
# are we in the right source dir ?
AC_CONFIG_SRCDIR([src/ggen.cpp])
# build tools in separate dir
AC_CONFIG_AUX_DIR([build-aux])
# macro in srcdir/m4
AC_CONFIG_MACRO_DIR([m4])

# configuration output in config.h
AC_CONFIG_HEADERS([config.h])

# automake should fail on any error
AM_INIT_AUTOMAKE([-Wall -Werror gnu])
# check for programs
AC_LANG_CPLUSPLUS
AC_PROG_CXX
AC_PROG_LIBTOOL
#check for GSL, see GSL manual for help
AC_CHECK_LIB([m],[cos],[],AC_MSG_ERROR("could not find the math library"))
AC_CHECK_LIB([gslcblas],[cblas_dgemm],[],AC_MSG_ERROR("could not find the GSL/CBLAS library"))
AC_CHECK_LIB([gsl],[gsl_blas_dgemm],[], AC_MSG_ERROR("could not find the GSL library"))

#check for BOOST
AX_BOOST_BASE([1.37])
AX_BOOST_GRAPH
if test "$BOOST_GRAPH_LIB" = ""; then
	AC_MSG_ERROR([No usable boost::graph found])
fi

DX_HTML_FEATURE(ON)
DX_CHM_FEATURE(OFF)
DX_CHI_FEATURE(OFF)
DX_MAN_FEATURE(OFF)
DX_RTF_FEATURE(OFF)
DX_XML_FEATURE(OFF)
DX_PDF_FEATURE(OFF)
DX_PS_FEATURE(OFF)
DX_INIT_DOXYGEN(ggen,doxygen.cfg,doxygen-doc)


# output makefiles
AC_CONFIG_FILES([Makefile src/Makefile lib/Makefile tests/Makefile doc/Makefile])
# do the output
AC_OUTPUT
