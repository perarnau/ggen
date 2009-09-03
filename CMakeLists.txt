# Copyright Swann Perarnau 2009
#
#   contributor(s) :  
#
#   contact : firstname.lastname@imag.fr	
#
# This software is a computer program whose purpose is to help the
# random generation of graph structures and adding various properties
# on those structures.
#
# This software is governed by the CeCILL  license under French law and
# abiding by the rules of distribution of free software.  You can  use, 
# modify and/ or redistribute the software under the terms of the CeCILL
# license as circulated by CEA, CNRS and INRIA at the following URL
# "http://www.cecill.info". 
# 
# As a counterpart to the access to the source code and  rights to copy,
# modify and redistribute granted by the license, users are provided only
# with a limited warranty  and the software's author,  the holder of the
# economic rights,  and the successive licensors  have only  limited
# liability. 
# 
# In this respect, the user's attention is drawn to the risks associated
# with loading,  using,  modifying and/or developing or reproducing the
# software by the user in light of its specific status of free software,
# that may mean  that it is complicated to manipulate,  and  that  also
# therefore means  that it is reserved for developers  and  experienced
# professionals having in-depth computer knowledge. Users are therefore
# encouraged to load and test the software's suitability as regards their
# requirements in conditions enabling the security of their systems and/or 
# data to be ensured and,  more generally, to use and operate it in the 
# same conditions as regards security. 
# 
# The fact that you are presently reading this means that you have had
# knowledge of the CeCILL license and that you accept its terms.

#######################################
## MAIN
#######################################

PROJECT(ggen CXX C)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

if(DEFINED GGEN_VERSION_STRING)
else(DEFINED GGEN_VERSION_STRING)
	message(STATUS "version string not defined, will use the devel name")
	set(GGEN_VERSION_STRING "devel")
endif(DEFINED GGEN_VERSION_STRING)

# set install path to /usr/local/ggen/*
set(CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/ggen")

# output binaries to the build/bin or build/lib dir
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin CACHE PATH "Single directory for all executables")
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib CACHE PATH "Single directory for all libraries")

mark_as_advanced(EXECUTABLE_OUTPUT_PATH LIBRARY_OUTPUT_PATH)
#######################################
## SUBDIRS
#######################################

# the source dir contains its own CMakeLists.txt with the
# real targets
add_subdirectory(src)

# the doc dir contains rules for doxygen generation
add_subdirectory(doc)

#######################################
## TESTING
#######################################

enable_testing()

#######################################
## PACKAGING
#######################################

# General info
set(CPACK_PACKAGE_NAME "ggen")
set(CPACK_PACKAGE_VENDOR "GGen development team")
set(CPACK_PACKAGE_VERSION ${GGEN_VERSION_STRING})

set(CPACK_GENERATOR "TGZ")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "GGen : a random graph generator")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README")
set(CPACK_PACKAGE_RESSOURCE_FILE_LICENCE "${CMAKE_SOURCE_DIR}/LICENCE")

# Binary package info
# TODO

# Source package info
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_IGNORE_FILES "/\\\\.svn/;\\\\.swp$;.*~;prototypes/;build/;www/;bibliography/;\\\\.git/;\\\\.bzr;\\\\.#;/#;cscope.*")
set(CPACK_SOURCE_INSTALLED_DIRECTORIES "${CMAKE_SOURCE_DIR}/;/")

include(CPack)
