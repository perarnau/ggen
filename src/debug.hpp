/* Copyright Swann Perarnau 2009
*
*   contributor(s) :  
*
*   contact : firstname.lastname@imag.fr	
*
* This software is a computer program whose purpose is to help the
* random generation of graph structures and adding various properties
* on those structures.
*
* This software is governed by the CeCILL  license under French law and
* abiding by the rules of distribution of free software.  You can  use, 
* modify and/ or redistribute the software under the terms of the CeCILL
* license as circulated by CEA, CNRS and INRIA at the following URL
* "http://www.cecill.info". 
* 
* As a counterpart to the access to the source code and  rights to copy,
* modify and redistribute granted by the license, users are provided only
* with a limited warranty  and the software's author,  the holder of the
* economic rights,  and the successive licensors  have only  limited
* liability. 
* 
* In this respect, the user's attention is drawn to the risks associated
* with loading,  using,  modifying and/or developing or reproducing the
* software by the user in light of its specific status of free software,
* that may mean  that it is complicated to manipulate,  and  that  also
* therefore means  that it is reserved for developers  and  experienced
* professionals having in-depth computer knowledge. Users are therefore
* encouraged to load and test the software's suitability as regards their
* requirements in conditions enabling the security of their systems and/or 
* data to be ensured and,  more generally, to use and operate it in the 
* same conditions as regards security. 
* 
* The fact that you are presently reading this means that you have had
* knowledge of the CeCILL license and that you accept its terms.
*/
/* GGen is a random graph generator :
 * it provides means to generate a graph following a
 * collection of methods found in the litterature.
 *
 * This is a research project founded by the MOAIS Team,
 * INRIA, Grenoble Universities.
 */

#ifndef DEBUG_H
#define DEBUG_H

/* Debug Levels */
#define DEBUG_LEVEL_info 0
#define DEBUG_LEVEL_warning 1
#define DEBUG_LEVEL_trace 2

#define DEBUG_DEFAULT_LEVEL DEBUG_LEVEL_info


#ifdef DEBUG
extern short dbg_level;
#define dbg(level,format,...) if(dbg_level > DEBUG_LEVEL_##level) \
		fprintf(stderr,"[file %s, line %d] "#level" : " format, __FILE__ , __LINE__ , ##__VA_ARGS__);


#define ADD_DBG_OPTIONS(x) x.add_options() \
				("debug,d",po::value<short>(&dbg_level),"Set the debug level")

#else

#define dbg(x,...) do { } while(0)

#define ADD_DBG_OPTIONS(x) do { } while(0)

#endif

#endif //DEBUG_H
