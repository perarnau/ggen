/* Copyright Swann Perarnau 2009
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

#ifndef BUILTIN_HPP
#define BUILTIN_HPP

#include "types.hpp"
#include "random.hpp"
#include "config.h"
using namespace ggen;

/* helps defining a table of all the commands known by a program*/
struct cmd_table_elt {
	const char* name;
	int (*fn)(int, char**);
};

struct help_elt {
	const char* name;
	const char** help;
};

/* C black magic to compute the size of a flexible array */
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

extern void die(const char *err,...);
extern void usage_helps(int argc, char **argv, struct help_elt helps[], int ask_full_help);
extern void usage(const char *help[]);

/* builtin commands external to ggen */
extern int cmd_generate_graph(int argc,char **argv);
extern int cmd_analyse_graph(int argc, char **argv);
extern int cmd_transform_graph(int argc, char**argv);
extern int cmd_add_property(int argc, char**argv);
extern int cmd_extract_property(int argc, char**argv);

#endif
