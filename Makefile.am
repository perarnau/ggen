ACLOCAL_AMFLAGS = -I m4
SUBDIRS = src tests doc

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = ggen.pc

README: README.md
	@PANDOC@ -s -t plain $< -o $@

all-local: README

clean-local:
	-rm README

EXTRA_DIST = autogen.sh version.sh ChangeLog ggen.pc
