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
## EXEC VARS
###################

# Executables

set(TESTS_EXECS lib_results)
###################
## EXEC RULES
###################

# add execs and link
foreach(it ${TESTS_EXECS})
	add_executable(${it} ${it}.cpp)
	target_link_libraries(${it} ${Boost_LIBRARIES} ${GSL_LIBRARIES} graph)
endforeach(it ${TESTS_EXECS})

###################
## INSTALL RULES
###################

# install commands
install(TARGETS ${TESTS_EXECS} 
	RUNTIME DESTINATION bin/)

###################
## TESTING RULES
###################
enable_testing()
# test the ggen_results
add_test(ggen_results ${EXECUTABLE_OUTPUT_PATH}/lib_results)

