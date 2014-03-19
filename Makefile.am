ACLOCAL_AMFLAGS = -I m4
SUBDIRS = src tests
if HAVE_PANDOC
SUBDIRS+=doc
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ggen.pc

EXTRA_DIST = autogen.sh version.sh ggen.pc README.markdown
