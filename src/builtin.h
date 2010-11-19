/* Copyright Swann Perarnau 2009
*
*   contact : Swann.Perarnau@imag.fr
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

#ifndef BUILTIN_H
#define BUILTIN_H
#include "ggen.h"

/* The ggen tool works on two levels:
 * - the first level define which part of ggen we are going to use
 *	it also handles input, output and rng initialization
 * - the second level is the real command to launch.
 *	those commands and their descriptions are saved in builtin-*
 *	files.
 */

/* since most commands share the use of graph and rng, they are
 * passed globally from one function to the other.
 */
extern igraph_t g;
extern igraph_t *g_p;
extern gsl_rng *rng;
extern FILE *infile;
extern FILE *outfile;
extern char *name;

#define EDGE_PROPERTY 0
#define VERTEX_PROPERTY 1
#define	GRAPH_PROPERTY 2
extern int ptype;

struct second_lvl_cmd {
	const char *name;
	unsigned int nargs;
	const char **help;
	int (*fn)(int,char**);
};

/* flags tell us which options
 * are possible with a command
 */
#define NEED_INPUT	1	// a graph needs read
#define NEED_OUTPUT	2	// a graph needs output
#define IS_GRAPH_P	4	// a graph pointer needs output
#define NEED_RNG	8	// a rng must be initialized
#define NEED_TYPE	16	// a type must be set
#define NEED_NAME	32	// a name must be set

struct first_lvl_cmd {
	const char *name;
	struct second_lvl_cmd *cmds;
	unsigned int flags;
	const char** help;
};

/* C black magic to compute the size of a flexible array */
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

/* second lvl commands and help arrays are provided
 * by builtin-* files
 */
extern const char *help_generate[];
extern const char *help_analyse[];
extern const char *help_transform[];
extern const char *help_add_prop[];
extern const char *help_analyse_prop[];

extern struct second_lvl_cmd cmds_generate[];
extern struct second_lvl_cmd cmds_analyse[];
extern struct second_lvl_cmd cmds_transform[];
extern struct second_lvl_cmd cmds_add_prop[];
extern struct second_lvl_cmd cmds_analyse_prop[];

#endif
