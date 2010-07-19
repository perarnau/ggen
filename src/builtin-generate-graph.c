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
#include <config.h>

/* globals needed by all commands */

static int ask_help = 0;
static int ask_full_help = 0;

igraph_t *g = NULL;
gsl_rng *rng = NULL;
FILE* out = NULL;

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

static const char* general_help[] = {
	"Usage: ggen generate-graph [options] method args\n\n",
	"Generic options:\n",
	"--help                   : ask for help. When a method is provided only display help for this method\n",
	"--full-help              : print the full help\n",
	"--output    <filename>   : specify a file for saved the graph\n",
	"\nRandom Numbers options:\n",
	"--rng-file  <filename>   : load and save the generator state in a specific file\n",
	"GSL_RNG_SEED             : use this environment variable to change the RNG seed\n",
	"GSL_RNG_TYPE             : use this environment variable to change the RNG type\n",
	"Look at the gsl documentation for more info.\n",
	"\nMethods available:\n",
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

/* Commands to handle */
static struct cmd_table_elt cmd_table[] = {
	{ "help", cmd_help, general_help, 0 },
	{ "gnp", cmd_gnp, gnp_help, 2 },
	{ "gnm", cmd_gnm, gnm_help, 2 },
	{ "lbl", cmd_lbl, lbl_help, 3 },
	{ "ro", cmd_ro, ro_help, 2 },
	{ "fifo" , cmd_fifo, fifo_help, 3},
};

static int cmd_help(int argc, char** argv)
{
	usage_helps(argc,argv,cmd_table,ask_full_help);
	return 0;
}

static int cmd_gnp(int argc, char** argv)
{
	int err = 0;
	unsigned long number;
	double prob;

	err = s2ul(argv[1],&number);
	if(err) goto ret;

	err = s2d(argv[2],&prob);
	if(err) goto ret;

	g = ggen_generate_erdos_gnp(rng,number,prob);
	if(g == NULL)
		err = 1;
ret:
	return err;
}

static int cmd_gnm(int argc, char** argv)
{
	int err = 0;
	unsigned long n,m;

	err = s2ul(argv[1],&n);
	if(err) goto ret;

	err = s2ul(argv[2],&m);
	if(err) goto ret;

	g = ggen_generate_erdos_gnm(rng,n,m);
	if(g == NULL)
		err = 1;
ret:
	return err;
}

static int cmd_lbl(int argc, char** argv)
{
	int err = 0;
	unsigned long n,l;
	double p;

	err = s2ul(argv[1],&n);
	if(err) goto ret;

	err = s2ul(argv[2],&l);
	if(err) goto ret;

	err = s2d(argv[3],&p);
	if(err) goto ret;

	g = ggen_generate_erdos_lbl(rng,n,l,p);
	if(g == NULL)
		err = 1;
ret:
	return err;
}

static int cmd_ro(int argc, char** argv)
{
	int err = 0;
	unsigned long n,o;

	err = s2ul(argv[1],&n);
	if(err) goto ret;

	err = s2ul(argv[2],&o);
	if(err) goto ret;

	g = ggen_generate_random_orders(rng,n,o);
	if(g == NULL)
		err = 1;
ret:
	return err;
}

static int cmd_fifo(int argc, char** argv)
{
	int err = 0;
	unsigned long n,i,o;

	err = s2ul(argv[1],&n);
	if(err) goto ret;

	err = s2ul(argv[2],&o);
	if(err) goto ret;

	err = s2ul(argv[2],&i);
	if(err) goto ret;

	g = ggen_generate_fifo(rng,n,o,i);
	if(g == NULL)
		err = 1;
ret:
	return err;
}

static int preop(char *rngfile)
{
	int status = 0;
	/* initialize gsl_rng */
	status = ggen_rng_init(&rng);
	if(status) return 1;

	/* load rng from file if possible */
	if(rngfile)
	{
		fprintf(stderr,"Using %s as RNG state file\n",rngfile);
		status = ggen_rng_load(&rng,rngfile);
		if(status)
			gsl_rng_free(rng);
	}

	return status;
}

static int postop(char *output,char *rngfile)
{
	int status = 0;
	if(output)
	{
		fprintf(stderr,"Using %s as output file\n",output);
		out = fopen(output,"w");
		if(!out)
		{
			fprintf(stderr,"failed to open file %s for graph output, using stdout instead\n",output);
			out = stdout;
			output = NULL;
		}
	}
	else
		out = stdout;

	status = ggen_write_graph(g,out);

	if(rngfile)
		status |= ggen_rng_save(&rng,rngfile);

	if(output)
		fclose(out);

	igraph_destroy(g);
	free(g);
	gsl_rng_free(rng);
	return status;
}


/* all command line arguments */
static struct option long_options[] = {
	/* general options */
	{ "help", no_argument, &ask_help, 1 },
	{ "full-help", no_argument, &ask_full_help, 1 },
	{ "output", required_argument, NULL, 'o' },
	/* random number generator */
	{ "rng-file", required_argument, NULL, 'f' },
	{ 0, 0, 0, 0},
};

static const char* short_opts = "ho:f:";

////////////////////////////////////////////////////////////////////////////////
// MAIN
////////////////////////////////////////////////////////////////////////////////

int cmd_generate_graph(int argc,char** argv)
{
	const char* cmd;
	int c;
	int option_index = 0;
	char *file = NULL;
	char *output = NULL;
	int status = 0;
	// parse other options
	while(1)
	{
		c = getopt_long(argc, argv, short_opts,long_options, &option_index);
		if(c == -1)
			break;

		switch(c)
		{
			case 'v':
			case 'h':
			case 0:
				break;
			case 'f':
				file = optarg;
				break;
			case 'o':
				output = optarg;
				break;
			case '?':
			default:
				exit(1);
		}
	}

	// now forget the parsed part of argv
	argc -= optind;
	argv = &(argv[optind]);

	// this is the command
	cmd = argv[0];
	// test for help
	if(!cmd || !strcmp(cmd,"help"))
	{
		status = cmd_help(argc,argv);
		goto ret;
	}

	for(int i = 1; i < ARRAY_SIZE(cmd_table); i++)
	{
		struct cmd_table_elt *c = cmd_table+i;
		if(!strcmp(c->name,cmd))
		{
			if(ask_help || c->nargs != argc -1)
			{
				usage(c->help);
				goto ret;
			}
			status = preop(file);
			if(status) goto ret;

			status = c->fn(argc,argv);
			if(status)
			{
				usage(c->help);
			}
			status |= postop(output,file);
			goto ret;
		}
	}
	fprintf(stderr,"Wrong command\n");
	status = 1;
ret:
	return status;
}
