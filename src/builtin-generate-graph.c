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

/* GGen is a random graph generator :
* it provides means to generate a graph following a
* collection of methods found in the litterature.
*
* This is a research project founded by the MOAIS Team,
* INRIA, Grenoble Universities.
*/

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <string.h>

#include "builtin.h"
#include "ggen.h"
#include "utils.h"
#include "config.h"

/* cmd declarations, we need this to be able to declare
 * the general struct
 */
static int cmd_help(int argc, char** argv);
static int cmd_gnp(int argc, char** argv);
static int cmd_gnm(int argc, char** argv);
static int cmd_lbl(int argc, char** argv);
static int cmd_fifo(int argc, char** argv);
static int cmd_ro(int argc, char** argv);

/* help strings, there is a lot of them */
const char* help_generate[] = {
	"Methods:\n",
	"gnp                      : the classical adjacency matrix method\n",
	"gnm                      : selection of edges in the complete graph\n",
	"lbl                      : the classical layer by layer method\n",
	"ro                       : generation of a DAG by intersection of total orders\n",
	"fifo                     : succeeding expension and contraction phases\n",
	NULL
};


static const char* gnp_help[] = {
	"\nErdos GNP:\n",
	"For each edge of the adjacency matrix, choose it with probability p.\n",
	"Arguments:\n",
	"     - number of vertices    : how many vertices in this graph\n",
	"     - probability           : the probability for each edge of the complete graph to be present\n",
	NULL
};

static const char* gnm_help[] = {
	"\nErdos GNM:\n",
	"Choose uniformly m edges in the complete adjacency matrix.\n",
	"Arguments:\n",
	"     - number of vertices    : how many vertices in this graph\n",
	"     - number of edges       : how many edges to choose\n",
	NULL
};

static const char* lbl_help[] = {
	"\nLayer By Layer:\n",
	"Split vertices into layers and connect layers between them.\n",
	"Arguments:\n",
	"     - number of vertices    : how many vertices in this graph\n",
	"     - number of layers      : how many layers to create\n",
	"     - probability           : chance to connect to vertices of different layers\n",
	NULL
};

static const char* ro_help[] = {
	"\nRandom Orders:\n",
	"Build a partial order from the intersection of random total orders and transform it into a DAG.\n",
	"Arguments:\n",
	"     - number of vertices    : how many vertices in this graph\n",
	"     - number of total orders: how many total orders should we intersect\n",
	NULL
};

static const char* fifo_help[] = {
	"\nFan-in / Fan-out:\n",
	"Build a graph by a succession of fan-in and fan-out steps.\n",
	"Arguments:\n",
	"     - number of vertices    : lower bound on the number of vertices wanted\n",
	"     - max out degree        : the maximum out degree of each vertex\n",
	"     - max in degree         : the maximum in degree of each vertex\n",
	NULL
};

struct second_lvl_cmd cmds_generate[] = {
	{ "gnp", 2, gnp_help, cmd_gnp },
	{ "gnm", 2, gnm_help, cmd_gnm },
	{ "lbl", 3, lbl_help, cmd_lbl },
	{ "ro", 2, ro_help, cmd_ro },
	{ "fifo" , 3, fifo_help, cmd_fifo },
	{ 0, 0, 0, 0},
};

static int cmd_gnp(int argc, char** argv)
{
	int err = 0;
	unsigned long number;
	double prob;

	err = s2ul(argv[0],&number);
	if(err) goto ret;

	err = s2d(argv[1],&prob);
	if(err) goto ret;

	g_p = ggen_generate_erdos_gnp(rng,number,prob);
	if(g_p == NULL)
		err = 1;
ret:
	return err;
}

static int cmd_gnm(int argc, char** argv)
{
	int err = 0;
	unsigned long n,m;

	err = s2ul(argv[0],&n);
	if(err) goto ret;

	err = s2ul(argv[1],&m);
	if(err) goto ret;

	g_p = ggen_generate_erdos_gnm(rng,n,m);
	if(g_p == NULL)
		err = 1;
ret:
	return err;
}

static int cmd_lbl(int argc, char** argv)
{
	int err = 0;
	unsigned long n,l;
	double p;

	err = s2ul(argv[0],&n);
	if(err) goto ret;

	err = s2ul(argv[1],&l);
	if(err) goto ret;

	err = s2d(argv[2],&p);
	if(err) goto ret;

	g_p = ggen_generate_erdos_lbl(rng,n,p,l);
	if(g_p == NULL)
		err = 1;
ret:
	return err;
}

static int cmd_ro(int argc, char** argv)
{
	int err = 0;
	unsigned long n,o;

	err = s2ul(argv[0],&n);
	if(err) goto ret;

	err = s2ul(argv[1],&o);
	if(err) goto ret;

	g_p = ggen_generate_random_orders(rng,n,o);
	if(g_p == NULL)
		err = 1;
ret:
	return err;
}

static int cmd_fifo(int argc, char** argv)
{
	int err = 0;
	unsigned long n,i,o;

	err = s2ul(argv[0],&n);
	if(err) goto ret;

	err = s2ul(argv[1],&o);
	if(err) goto ret;

	err = s2ul(argv[2],&i);
	if(err) goto ret;

	g_p = ggen_generate_fifo(rng,n,o,i);
	if(g_p == NULL)
		err = 1;
ret:
	return err;
}
