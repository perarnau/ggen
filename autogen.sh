#!/bin/sh
set -x
# auto* need build-aux directory
mkdir -p build-aux || exit 1
sh version.sh
aclocal -I m4 || exit 1
autoheader || exit 1
libtoolize || exit 1
automake --add-missing --copy || exit 1
autoconf || exit 1
