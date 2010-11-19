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

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <getopt.h>

#include "builtin.h"
#include "ggen.h"
#include "config.h"
#include "utils.h"

static const char * general_help[] = {
	"Usage: ggen [options] <cmd> <args>\n\n",
	"Allowed options:\n",
	"--help,-h               : print this help message\n",
	"--full-help             : display as much help as possible\n",
	"--version,-V            : print program version\n",
	"--input,-i   <file>     : use file as input, default to stdin\n",
	"--output,-o  <file>     : use file as output, default to stdout\n",
	"--rng-file   <file>     : use file to save rng state\n",
	"                          if possible, will load state before cmd\n"
	"--edge                  : manipulate an edge property\n",
	"--vertex                : manipulate a vertex property \n",
	"--name       <string>   : use string as name\n",
	"NOTE: most of these options are only available on some commands\n",
	"\nEnvironment Variables:\n",
	"GSL_RNG_SEED             : use this environment variable to change the RNG seed\n",
	"GSL_RNG_TYPE             : use this environment variable to change the RNG type\n",
	"Look at the gsl documentation for more info.\n",
	"\nCommands available:\n",
	"generate-graph          : use the graph utils\n",
	"analyse-graph           : use the graph analysis tools\n",
	"transform-graph         : use the graph transformation tools\n",
	"add-property            : use the property adding tools\n",
	"analyse-property        : extract a property from the graph\n",
	NULL
};

static struct first_lvl_cmd cmd_table[] = {
	{ "generate-graph" , cmds_generate, NEED_OUTPUT | IS_GRAPH_P | NEED_RNG, help_generate },
	{ "analyse-graph", cmds_analyse, NEED_INPUT, help_analyse },
	{ "transform-graph", cmds_transform, NEED_INPUT | NEED_OUTPUT, help_transform },
	{ "add-property", cmds_add_prop, NEED_INPUT | NEED_OUTPUT | NEED_RNG | NEED_NAME | NEED_TYPE, help_add_prop },
	{ "analyse-property", cmds_analyse_prop, NEED_INPUT | NEED_NAME | NEED_TYPE, help_analyse_prop },
};

static const char *ggen_version_string = PACKAGE_STRING;

static int ask_help = 0;
static int ask_full_help = 0;
static int ask_version = 0;
static char* rngfname = NULL;
static char* infname = NULL;
static char* outfname = NULL;

/* global variables */
igraph_t g;
igraph_t *g_p = NULL;
gsl_rng *rng = NULL;
FILE *infile = NULL;
FILE *outfile = NULL;
char *name = NULL;
int ptype = -1;

/* all command line arguments */
static struct option long_options[] = {
	/* general options */
	{ "help", no_argument, &ask_help, 1 },
	{ "full-help", no_argument, &ask_full_help, 1 },
	{ "version", no_argument, &ask_version, 1 },
	{ "input", required_argument, NULL, 'i' },
	{ "output", required_argument, NULL, 'o' },
	/* random number generator */
	{ "rng-file", required_argument, NULL, 'r' },
	/* properties options */
	{ "name", required_argument, NULL, 'n' },
	{ "edge", no_argument, &ptype, EDGE_PROPERTY },
	{ "vertex", no_argument, &ptype, VERTEX_PROPERTY },
	{ 0, 0, 0, 0},
};

static const char* short_opts = "hVi:o:r:n:";

void print_help(const char **message) {
	for(int i=0; message[i] != NULL; i++)
		fprintf(stdout,"%s",message[i]);
}

/* use flags to display which options are valid and
 * print the full help of each subcommand
 */
void print_first_lvl_help(struct first_lvl_cmd *fl)
{
	fprintf(stdout,"\n%s:\n\n",fl->name);
	fprintf(stdout,"Valid Options: output");
	if(fl->flags & NEED_INPUT)
		fprintf(stdout,", input");
	if(fl->flags & NEED_RNG)
		fprintf(stdout,", rng");
	if(fl->flags & NEED_TYPE)
		fprintf(stdout,", edge, vertex");
	if(fl->flags & NEED_NAME)
		fprintf(stdout,", name");
	fprintf(stdout,"\n");
	print_help(fl->help);
	for(int i = 0; fl->cmds[i].name != NULL; i++)
		if(fl->cmds[i].help != NULL)
			print_help(fl->cmds[i].help);
}

void print_full_help() {
	print_help(general_help);
	for(int i = 0; i < ARRAY_SIZE(cmd_table); i++)
	{
		struct first_lvl_cmd *fl = cmd_table+i;
		print_first_lvl_help(fl);
	}
}

void print_version()
{
	fprintf(stdout,"ggen: version %s\n",ggen_version_string);
}

int handle_second_lvl(int argc,char **argv,struct first_lvl_cmd *fl, struct second_lvl_cmd *sl)
{
	int status = 0;
	// check for help
	if(ask_help || (argc == 0 && sl->nargs != 0))
	{
		if(sl->help != NULL)
			print_help(sl->help);
		else
			print_first_lvl_help(fl);
		return 0;
	}
	// check number of arguments
	if(argc != sl->nargs)
	{
		fprintf(stderr,"error: wrong number of arguments");
		return 1;
	}
	// open input
	if(fl->flags & NEED_INPUT)
	{
		if(infname)
		{
			fprintf(stderr,"Using %s as input file\n",infname);
			infile = fopen(infname,"r");
			if(!infile)
			{
				fprintf(stderr,"failed to open file %s for graph input, using stdin instead\n",infname);
				infile = stdin;
				infname = NULL;
			}
		}
		else
			infile = stdin;

		status = ggen_read_graph(&g,infile);
		if(infname)
			fclose(infile);
		if(status)
		{
			fprintf(stderr,"error during graph reading\n");
			goto free_ing;
		}
	}
	// load rng
	if(fl->flags & NEED_RNG)
	{
		status = ggen_rng_init(&rng);
		if(status)
		{
			fprintf(stderr,"error during rng init\n");
			goto free_ing;
		}
		if(rngfname)
		{
			fprintf(stderr,"Using %s as RNG state file\n",rngfname);
			status = ggen_rng_load(&rng,rngfname);
			if(status == 1)
				fprintf(stderr,"failed to read rng state, will continue anyway\n");
			else if(status != 0)
			{
				fprintf(stderr,"error during rng read, will stop\n");
				goto free_rng;
			}
		}

	}
	// set name
	if((fl->flags & NEED_NAME) && name == NULL)
	{
		name = "newproperty";
	}
	// set type
	if((fl->flags & NEED_TYPE) && ptype == -1)
	{
		ptype = VERTEX_PROPERTY;
	}

	// output is a bit different from input:
	// a command can have its output redirected even
	// if it does not generate a graph
	// need_output tells us if a resulting graph needs
	// to be wrote, not if the output can be redirected
	if(outfname)
	{
		fprintf(stderr,"Using %s as output file\n",outfname);
		outfile = fopen(outfname,"w");
		if(!outfile)
		{
			fprintf(stderr,"failed to open file %s for graph output, using stdout instead\n",outfname);
			outfile = stdout;
			outfname = NULL;
		}
	}
	else
		outfile = stdout;

	// launch cmd
	status = sl->fn(argc,argv);
	if(status)
	{
		fprintf(stderr,"error during command\n");
		goto err;
	}

	if(fl->flags & NEED_OUTPUT)
	{
		if(fl->flags & IS_GRAPH_P)
			status = ggen_write_graph(g_p,outfile);
		else
			status = ggen_write_graph(&g,outfile);

		if(outfname)
			fclose(outfile);

		if(status)
		{
			fprintf(stderr,"error during graph saving\n");
			goto free_outg;
		}
	}
	if((fl->flags & NEED_RNG) && rngfname)
	{
		status = ggen_rng_save(&rng,rngfname);
		if(status)
		{
			fprintf(stderr,"error during rng save\n");
		}
	}
	if(outfname)
	{
		fclose(outfile);
	}
free_outg:
	if(fl->flags & IS_GRAPH_P)
	{
		igraph_destroy(g_p);
		free(g_p);
	}
err:
free_rng:
	if(fl->flags & NEED_RNG)
		gsl_rng_free(rng);
free_ing:
	if(fl->flags & NEED_INPUT)
		igraph_destroy(&g);
	return status;
}


int handle_first_lvl(int argc, char **argv, struct first_lvl_cmd *c)
{
	int status = 0;
	if(argc == 0)
	{
		print_first_lvl_help(c);
		return 0;
	}
	// check that user didn't ask for something crazy
	if(infname != NULL && !(c->flags & NEED_INPUT))
	{
		fprintf(stderr,"error: I don't need an input file\n");
		return 1;
	}
	if(name != NULL && !(c->flags & NEED_NAME))
	{
		fprintf(stderr,"error: I don't need an name\n");
		return 1;
	}
	if(ptype != -1 && !(c->flags & NEED_TYPE))
	{
		fprintf(stderr,"error: I don't need an property type\n");
		return 1;
	}
	if(rngfname != NULL && !(c->flags & NEED_RNG))
	{
		fprintf(stderr,"error: I don't need an rng file\n");
		return 1;
	}
	// find second lvl command
	for(int j = 0; c->cmds[j].name != NULL; j++)
	{
		struct second_lvl_cmd *sl = c->cmds+j;
		if(!strcmp(sl->name,argv[0]))
		{
			argc--;
			argv++;
			status = handle_second_lvl(argc,argv,c,sl);
			return status;
		}
	}
	// no valid subcmd
	fprintf(stderr,"error:wrong cmd\n");
	return 1;
}




int main(int argc,char** argv)
{
	int c;
	int option_index = 0;
	int status = 0;
	// parse options
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
				rngfname = optarg;
				break;
			case 'o':
				outfname = optarg;
				break;
			case 'i':
				infname = optarg;
				break;
			case 'n':
				name = optarg;
				break;
			case 'h':
				ask_help = 1;
				break;
			case 'V':
				ask_version =1;
				break;
			default:
				fprintf(stderr,"ggen bug: someone forgot how to write a switch\n");
				exit(EXIT_FAILURE);
			case '?':
				fprintf(stderr,"ggen bug: getopt failed miserably\n");
				exit(EXIT_FAILURE);
		}
	}
	// forget the parsed part of argv
	argc -= optind;
	argv = &(argv[optind]);

	if(ask_full_help)
	{
		print_full_help();
		exit(EXIT_SUCCESS);
	}
	if(ask_version)
	{
		print_version();
		exit(EXIT_SUCCESS);
	}
	if(argc == 0)
	{
		print_help(general_help);
		exit(EXIT_SUCCESS);
	}

	// initialize igraph attributes for all commands
	igraph_i_set_attribute_table(&igraph_cattribute_table);

	// find the command to launch
	for(int i = 0; i < ARRAY_SIZE(cmd_table); i++)
	{
		struct first_lvl_cmd *c = cmd_table+i;
		if(!strcmp(c->name,argv[0]))
		{
			argc--;
			argv++;
			status = handle_first_lvl(argc,argv,c);
			return status;
		}
	}
	fprintf(stderr,"error: wrong command\n");
	exit(EXIT_FAILURE);
}
