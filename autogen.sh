#!/bin/sh

set -x
aclocal -I m4
autoheader
libtoolize
automake --add-missing --copy
autoconf
