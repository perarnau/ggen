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

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake/")

# For boost
SET(Boost_USE_STATIC_LIBS OFF)
SET(Boost_USE_MULTITHREAD OFF)
FIND_PACKAGE(Boost REQUIRED regex program_options graph)

# For GSL
find_package(GSL)

# remove deprecated warning about hash_set with gcc 4.3
ADD_DEFINITIONS(-DBOOST_NO_HASH)
# Some flags definitions
set(CMAKE_CXX_FLAGS_DEBUG	" -g -O2 -DDEBUG ")
set(CMAKE_C_FLAGS_DEBUG       " -g -O2 -DDEBUG ")
set(CMAKE_CXX_FLAGS_RELEASE       "-O2")
set(CMAKE_C_FLAGS_RELEASE       "-O2")


# Executables
set(EXECS_WITH_RANDOM graph_generator property_adder)
set(EXECS graph_analysis graph_transform property_analysis)

# commom files dependencies

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
set(COMMON_DEPS dynamic_properties.cpp debug.cpp)
else(true)
set(COMMON_DEPS dynamic_properties.cpp)
endif(CMAKE_BUILD_TYPE STREQUAL "Debug")

# add execs and link
foreach(it ${EXECS_WITH_RANDOM})
	add_executable(${it} ${it}.cpp ${COMMON_DEPS} random.cpp)
	target_link_libraries(${it} ${Boost_LIBRARIES} ${GSL_LIBRARIES})
endforeach(it ${EXECS_WITH_RANDOM})

foreach(it ${EXECS})
	add_executable(${it} ${it}.cpp ${COMMON_DEPS})
	target_link_libraries(${it} ${Boost_LIBRARIES} ${GSL_LIBRARIES})
endforeach(it ${EXECS})


#######################################
## PACKAGING
#######################################

# General info
set(CPACK_PACKAGE_NAME "ggen")
set(CPACK_PACKAGE_VENDOR "GGen development team")
set(CPACK_PACKAGE_VERSION "0.1.a")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "a")

set(CPACK_GENERATOR "TGZ")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "GGen : a random graph generator")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/../README")
set(CPACK_PACKAGE_RESSOURCE_FILE_LICENCE "${CMAKE_SOURCE_DIR}/../LICENCE")

# Binary package info
# TODO

# Source package info
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_IGNORE_FILES "/\\\\.svn/;\\\\.swp$;.*~;prototypes/;build/;www/;docs/;\\\\.git/;\\\\.bzr;\\\\.#;/#;cscope.*")
set(CPACK_SOURCE_INSTALLED_DIRECTORIES "${CMAKE_SOURCE_DIR}/../;/")

include(CPack)
