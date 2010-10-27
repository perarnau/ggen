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

static int ask_help = 0;
static int ask_full_help = 0;

static int is_edge = 0, is_vertex = 0;

static char* infile = NULL;
static FILE *in;

static char* outfile = NULL;
static FILE *out;

static char *prop = NULL;

static igraph_t g;
static gsl_rng *rng = NULL;

static const char* general_help[] = {
	"Usage: ggen add-property [options] cmd args\n\n",
	"Generic options:\n",
	"--help                   : ask for help. When a method is provided only display help for this method\n",
	"--full-help              : display the full help message, including a detailled description of each command\n",
	"--input     <filename>   : specify an input file for the graph\n",
	"--output    <filename>   : specify a file for saved the graph\n",
	"\nProperty options:\n",
	"--name                   : the name of the new property\n",
	"--edge,--vertex          : force the type of property to add\n",
	"\nRandom Numbers options:\n",
	"--rng-file  <filename>   : load and save the generator state in a specific file\n",
	"\nCommands available:\n",
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


static int cmd_help(int argc, char** argv);
static int cmd_exponential(int argc, char** argv);
static int cmd_gaussian(int argc, char** argv);
static int cmd_flat(int argc, char** argv);
static int cmd_pareto(int argc, char** argv);

/* Commands to handle */
static struct cmd_table_elt cmd_table[] = {
	{ "help", cmd_help, general_help, 0 },
	{ "exponential", cmd_exponential , exponential_help, 1},
	{ "gaussian", cmd_gaussian, gaussian_help, 1 },
	{ "flat", cmd_flat, flat_help, 2 },
	{ "pareto", cmd_pareto, pareto_help, 2 },
};

// pre processing operations
static int preop()
{
	int err;
	if(infile)
	{
		fprintf(stderr,"Using %s as input file\n",infile);
		in = fopen(infile,"r");
		if(!in)
		{
			fprintf(stderr,"failed to open file %s for graph input, using stdin instead\n",infile);
			in = stdout;
			infile = NULL;
		}
	}
	else
		in = stdin;
	err = ggen_read_graph(&g,in);
	if(infile)
		fclose(in);
	return err;
}

// post processing operations
static int postop()
{
	int err;
	if(outfile)
	{
		fprintf(stderr,"Using %s as output file\n",outfile);
		out = fopen(outfile,"w");
		if(!out)
		{
			fprintf(stderr,"failed to open file %s for graph output, using stdout instead\n",outfile);
			out = stdout;
			outfile = NULL;
		}
	}
	else
		out = stdout;
	err = ggen_write_graph(&g,out);
	if(outfile)
		fclose(out);
	return err;
}

/**
 * macro defining cmd_functions to call create rnds
 * needs a help struct name_help and a ggen_rnd_name
 * 1 double argument version
 */
#define DEFINE_CMD_1D(name)				\
static int cmd_##name(int argc, char **argv)		\
{							\
	int err;					\
	double arg;					\
	unsigned long count,i;				\
							\
	err = s2d(argv[1],&arg);			\
	if(err) return 1;				\
							\
	if(is_edge)					\
		count = igraph_ecount(&g);		\
	else						\
		count = igraph_vcount(&g);		\
							\
	for(i = 0; i < count; i++) {			\
		if(is_edge)				\
			SETEAN(&g,prop,i,gsl_ran_##name(rng,arg));\
		else					\
			SETVAN(&g,prop,i,gsl_ran_##name(rng,arg));\
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
#define DEFINE_CMD_2D(name)				\
static int cmd_##name(int argc, char **argv)		\
{							\
	int err;					\
	double arg1,arg2;				\
	unsigned long count,i;				\
							\
	err = s2d(argv[1],&arg1);			\
	if(err) return 1;				\
							\
	err = s2d(argv[1],&arg2);			\
	if(err) return 1;				\
							\
	if(is_edge)					\
		count = igraph_ecount(&g);		\
	else						\
		count = igraph_vcount(&g);		\
							\
	for(i = 0; i < count; i++) {			\
		if(is_edge)				\
			SETEAN(&g,prop,i,gsl_ran_##name(rng,arg1,arg2));\
		else					\
			SETVAN(&g,prop,i,gsl_ran_##name(rng,arg1,arg2));\
	}						\
							\
	return err;					\
}

DEFINE_CMD_2D(flat)
DEFINE_CMD_2D(pareto)

static int cmd_help(int argc, char** argv)
{
	usage_helps(argc,argv,cmd_table,ask_full_help);
	return 0;
}


/* all command line arguments */
static struct option long_options[] = {
	/* general options */
	{ "help", no_argument, &ask_help, 1 },
	{ "full-help", no_argument, &ask_full_help, 1 },
	{ "input", required_argument, NULL, 'i' },
	{ "output", required_argument, NULL, 'o' },
	/* property options */
	{ "name", required_argument, NULL, 'n' },
	{ "edge", no_argument, &is_edge, 1 },
	{ "vertex", no_argument, &is_vertex, 1 },
	/* random number generator */
	{ "rng-file", required_argument, NULL, 'f' },
	{ 0, 0, 0, 0},
};

static const char* short_opts = "i:o:n:f:";

/**
* Main program
*
*/
int cmd_add_property(int argc,char** argv)
{
	const char* cmd;
	int c;
	int option_index = 0;
	char *file = NULL;
	int status = 0;
	// parse other options
	while(1)
	{
		c = getopt_long(argc, argv, short_opts,long_options, &option_index);
		if(c == -1)
			break;

		switch(c)
		{
			case 0:
				break;
			case 'f':
				file = optarg;
				break;
			case 'o':
				outfile = optarg;
				break;
			case 'i':
				infile = optarg;
				break;
			case 'n':
				prop = optarg;
				break;
			default:
				die("someone forgot how to write switches");
			case '?':
				die("option parsing got mad");
		}
	}

	if(is_edge && is_vertex)
	{
		die("Cannot specify --edge and --vertex !");
	}

	// now forget the parsed part of argv
	argc -= optind;
	argv = &(argv[optind]);

	// this is the command
	cmd = argv[0];
	if(!cmd || !strcmp("help",cmd))
	{
		status = cmd_help(argc,argv);
		goto ret;
	}

	// init rng
	status = ggen_rng_init(&rng);
	if(status) goto ret;

	/* load rng from file if possible */
	if(file)
	{
		fprintf(stderr,"Using %s as RNG state file\n",file);
		status = ggen_rng_load(&rng,file);
		if(status) goto cleanup;
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

			status = preop();
			if(status) goto ret;

			status = c->fn(argc,argv);

			if(status)
			{
				usage(c->help);
				goto cleanup;
			}
			status = postop();
			goto cleanup;
		}
	}
	fprintf(stderr,"Wrong command\n");
	status = 1;
	goto cleanup;
out:
	if(file)
		status = ggen_rng_save(&rng,file);

cleanup:
	igraph_destroy(&g);
	gsl_rng_free(rng);
ret:
	return status;

}
