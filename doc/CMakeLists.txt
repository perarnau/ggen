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

# For Doxygen
find_package(Doxygen)

###################
## MAIN
###################

# Ok this is a bit complicated :
# doxygen is normally run from the binary directory.
# Then doxygen is not very smart and any relative path in the
# doxyfile is relative to the cwd of doxygen not of the file.
# So the best way to really compile the docs in the build dir :
#  - have a doxyfile.in that contains our doxygen config + cmake variables
#  - use configure_file to copy this file to the build dir and replace the cmake variables
#  - execute doxygen
if(DOXYGEN_FOUND)

set(DOXYFILE_IN "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in")
configure_file(${DOXYFILE_IN} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

add_custom_target(doxygen ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

add_custom_target(doc)

add_dependencies(doc doxygen)

###################
## INSTALL RULES
###################

# install commands
install(DIRECTORY doc DESTINATION doc)

endif()
