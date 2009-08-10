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

###################
## DEPENDENIES
###################
# For boost
SET(Boost_USE_STATIC_LIBS OFF)
SET(Boost_USE_MULTITHREAD OFF)
FIND_PACKAGE(Boost REQUIRED program_options graph)

# For GSL
find_package(GSL)

###################
## DEFINITIONS
###################

add_definitions(-DGGEN_VERSION_STRING=\"${GGEN_VERSION_STRING}\")

# remove deprecated warning about hash_set with gcc 4.3
ADD_DEFINITIONS(-DBOOST_NO_HASH)

# Some flags definitions
set(CMAKE_CXX_FLAGS_DEBUG	" -g -O2 -DDEBUG ")
set(CMAKE_C_FLAGS_DEBUG       	" -g -O2 -DDEBUG ")
set(CMAKE_CXX_FLAGS_RELEASE	"-O2")
set(CMAKE_C_FLAGS_RELEASE	"-O2")

###################
## LIB VARS
###################

set(LIB_FILES dynamic_properties.cpp graph-generation.cpp graph-analysis.cpp random.cpp graph-transformation.cpp graph-properties.cpp)

add_library(graph STATIC ${LIB_FILES})


###################
## EXEC VARS
###################

# Executables
set(EXECS ggen)

set(EXECS_FILES builtin-generate-graph.cpp ggen.cpp builtin-analyse-graph.cpp builtin-transform-graph.cpp builtin-add-property.cpp builtin-extract-property.cpp usage.cpp)

###################
## EXEC RULES
###################

add_executable(ggen ${EXECS_FILES})
target_link_libraries(ggen ${Boost_LIBRARIES} ${GSL_LIBRARIES} graph)

###################
## INSTALL RULES
###################

# install commands
install(TARGETS ggen
	RUNTIME DESTINATION bin)

###################
## TESTING 
###################

enable_testing()
set(commands analyse-graph generate-graph transform-graph add-property extract-property)
## basic tests for help message
foreach(it ${commands})
	add_test(usage-${it} ${EXECUTABLE_OUTPUT_PATH}/ggen ${it})
	set_property(TEST usage-${it} PROPERTY TIMEOUT 2)
	set_property(TEST usage-${it} PROPERTY PASS_REGULAR_EXPRESSION "Usage")
endforeach(it ${commands})

###################
## SCRIPTS
###################

add_subdirectory(scripts)
