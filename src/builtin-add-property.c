/* Copyright Swann Perarnau 2009
*
*   contributor(s) :
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

/* GGen is a random graph generator :
* it provides means to generate a graph following a
* collection of methods found in the litterature.
*
* This is a research project founded by the MOAIS Team,
* INRIA, Grenoble Universities.
*/

#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <gsl/gsl_randist.h>

#include "builtin.h"
#include "ggen.h"
#include "utils.h"

const char* help_add_prop[] = {
	"Commands:\n",
	"gaussian                 : add a property following a gaussian distribution\n",
	"flat                     : add a property following a flat (uniform) distribution\n",
	"exponential              : add a property following an exponential distribution\n",
	"pareto                   : add a property following a pareto distribution\n",
	NULL
};

static const char* exponential_help[] = {
	"\nExponential Distribution:\n",
	"Use an exponential distribution as the random number generator for the property.\n",
	"Arguments:\n",
	"     - mu             : the distribution will have a mean of this value\n",
	NULL
};

static const char* gaussian_help[] = {
	"\nGaussian Distribution:\n",
	"Use a gaussian distribution as the random number generator for the property.\n",
	"Arguments:\n",
	"     - sigma             : the distribution will be centered on this value\n",
	NULL
};

static const char* flat_help[] = {
	"\nFlat Distribution:\n",
	"Use a flat (uniform) distribution as the random number generator for the property.\n",
	"Arguments:\n",
	"     - min               : minimum value included in the distribution\n",
	"     - max               : maximum value included in the distribution\n",
	NULL
};

static const char* pareto_help[] = {
	"\nPareto Distribution:\n",
	"Use a pareto distribution as the random number generator for the property.\n",
	"Arguments:\n",
	"     - a               : order of the distribution\n",
	"     - b               : minimum value of the distribution\n",
	NULL
};


static int cmd_exponential(int argc, char** argv);
static int cmd_gaussian(int argc, char** argv);
static int cmd_flat(int argc, char** argv);
static int cmd_pareto(int argc, char** argv);

/* Commands to handle */
struct second_lvl_cmd cmds_add_prop[] = {
	{ "exponential", 1, exponential_help, cmd_exponential},
	{ "gaussian", 1, gaussian_help, cmd_gaussian},
	{ "flat", 2, flat_help, cmd_flat},
	{ "pareto", 2, pareto_help, cmd_pareto},
	{ 0, 0, 0, 0},
};

/**
 * macro defining cmd_functions to call create rnds
 * needs a help struct name_help and a ggen_rnd_name
 * 1 double argument version
 */
#define DEFINE_CMD_1D(dist)				\
static int cmd_##dist(int argc, char **argv)		\
{							\
	int err;					\
	double arg;					\
	unsigned long count,i;				\
							\
	err = s2d(argv[0],&arg);			\
	if(err) return 1;				\
							\
	if(ptype == EDGE_PROPERTY)			\
		count = igraph_ecount(&g);		\
	else						\
		count = igraph_vcount(&g);		\
							\
	for(i = 0; i < count; i++) {			\
		if(ptype == EDGE_PROPERTY)		\
			SETEAN(&g,name,i,gsl_ran_##dist(rng,arg));\
		else					\
			SETVAN(&g,name,i,gsl_ran_##dist(rng,arg));\
	}						\
							\
	return err;					\
}

DEFINE_CMD_1D(exponential)
DEFINE_CMD_1D(gaussian)

/**
 * macro defining cmd_functions to call create rnds
 * needs a help struct name_help and a ggen_rnd_name
 * 2 double arguments version
 */
#define DEFINE_CMD_2D(dist)				\
static int cmd_##dist(int argc, char **argv)		\
{							\
	int err;					\
	double arg1,arg2;				\
	unsigned long count,i;				\
							\
	err = s2d(argv[0],&arg1);			\
	if(err) return 1;				\
							\
	err = s2d(argv[1],&arg2);			\
	if(err) return 1;				\
							\
	if(ptype == EDGE_PROPERTY)			\
		count = igraph_ecount(&g);		\
	else						\
		count = igraph_vcount(&g);		\
							\
	for(i = 0; i < count; i++) {			\
		if(ptype == EDGE_PROPERTY)		\
			SETEAN(&g,name,i,gsl_ran_##dist(rng,arg1,arg2));\
		else					\
			SETVAN(&g,name,i,gsl_ran_##dist(rng,arg1,arg2));\
	}						\
							\
	return err;					\
}

DEFINE_CMD_2D(flat)
DEFINE_CMD_2D(pareto)
