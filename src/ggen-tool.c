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
	"--log-file,-f   <file>  : use file for logging\n",
	"--log-level,-l  <int>   : set verbosity for logging\n",
	"                          0 is quiet, 5 is debugging\n",
	"--input,-i      <file>  : use file as input, default to stdin\n",
	"--output,-o     <file>  : use file as output, default to stdout\n",
	"--rng-file,-r   <file>  : use file to save rng state\n",
	"                          if possible, will load state before cmd\n"
	"--edge                  : manipulate an edge property\n",
	"--vertex                : manipulate a vertex property\n",
	"--graph                 : manipulate a graph property\n",
	"--name       <string>   : use string as name\n",
	"NOTE: most of these options are only available on some commands\n",
	"\nEnvironment Variables:\n",
	"GSL_RNG_SEED             : use this environment variable to change the RNG seed\n",
	"GSL_RNG_TYPE             : use this environment variable to change the RNG type\n",
	"Look at the gsl documentation for more info.\n",
	"\nCommands available:\n",
	"generate-graph          : generate random graphs\n",
	"static-graph            : generate static graphs\n",
	"analyse-graph           : use the graph analysis tools\n",
	"transform-graph         : use the graph transformation tools\n",
	"add-property            : use the property adding tools\n",
	"analyse-property        : extract a property from the graph\n",
	NULL
};

static struct first_lvl_cmd cmd_table[] = {
	{ "generate-graph" , cmds_generate, NEED_OUTPUT | IS_GRAPH_P | NEED_RNG, help_generate },
	{ "static-graph" , cmds_static, NEED_OUTPUT | IS_GRAPH_P,  help_static },
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

/* logging variables */
static char* logfname = NULL;
static FILE* logfile = NULL;
static char* logval = NULL;

/*========= LOGGING POLICY ===========
 * GGen now integrates logging facilities.
 * This paragraph explains how ggen log events
 * and how it should always be implemented.
 *
 * Logging should be activated as soon as possible.
 * It should only replace output on stderr. No display
 * targetted for stdout should be changed: this includes
 * command results and legitimate help (those that are asked).
 *
 * Error in the program should go to logging level ERROR.
 * NORMAL level includes any trace of the command asked and
 * operation realized.
 * WARNING is for special behaviors: not failling on rng-file
 * read for example.
 * INFO adds values information about commands
 * DEBUG should be used by developers only and should not
 * be included in release code.
 */


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
	{ "log-file", required_argument, NULL, 'f' },
	{ "log-level", required_argument, NULL, 'l'},
	/* random number generator */
	{ "rng-file", required_argument, NULL, 'r' },
	/* properties options */
	{ "name", required_argument, NULL, 'n' },
	{ "edge", no_argument, &ptype, EDGE_PROPERTY },
	{ "vertex", no_argument, &ptype, VERTEX_PROPERTY },
	{ "graph", no_argument, &ptype, GRAPH_PROPERTY },
	{ 0, 0, 0, 0},
};

static const char* short_opts = ":hVi:o:r:n:f:l:";

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
		fprintf(stdout,", edge, vertex, graph");
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

/* helper function for command flags */
int handle_need_input(void)
{
	int status;
	igraph_bool_t isdag;

	normal("Configuring input\n");
	if(infname)
	{
		info("Using %s as input file\n",infname);
		infile = fopen(infname,"r");
		if(!infile)
		{
			warning("failed to open file %s for graph input, using stdin instead\n",infname);
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
		error("Failed to read graph\n");
		return 1;
	}
	/* check that the graph is a DAG */
	status = igraph_is_dag(&g,&isdag);
	if(status || !isdag)
	{
		error("Input graph failed DAG verification\n");
		return 1;
	}
	normal("Input configured and graph read\n");
	return 0;
}

int handle_need_rng(void)
{
	int status;
	normal("Configuring random number generator\n");
	// turn off automatic abort on gsl error
	gsl_set_error_handler_off();
	status = ggen_rng_init(&rng);
	if(status)
	{
		error("Failed to initialize RNG\n");
		return 1;
	}
	if(rngfname)
	{
		info("Using %s as RNG state file\n",rngfname);
		status = ggen_rng_load(&rng,rngfname);
		if(status == 1)
			warning("RNG State file not found, will continue anyway\n");
		else if(status != 0)
		{
			error("Reading RNG State from file failed.\n");
			return 1;
		}
	}
	normal("RNG configured\n");
	return 0;
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
		info("Expected %u arguments, found %u\n",sl->nargs,argc);
		error("Wrong number of arguments\n");
		return 1;
	}
	// open input
	if(fl->flags & NEED_INPUT)
	{
		if(handle_need_input())
			return 1;
	}
	// load rng
	if(fl->flags & NEED_RNG)
	{
		status = handle_need_rng();
		if(status)
			goto free_ing;
	}
	// set name
	if((fl->flags & NEED_NAME) && name == NULL)
	{
		name = "newproperty";
		info("Property name needed, using %s as default\n",name);
	}
	// set type
	if((fl->flags & NEED_TYPE) && ptype == -1)
	{
		ptype = VERTEX_PROPERTY;
		info("Property type needed, using VERTEX as default\n");
	}

	// output is a bit different from input:
	// a command can have its output redirected even
	// if it does not generate a graph
	// need_output tells us if a resulting graph needs
	// to be wrote, not if the output can be redirected
	normal("Configuring output\n");
	if(outfname)
	{
		info("Opening %s for writing\n",outfname);
		outfile = fopen(outfname,"w");
		if(!outfile)
		{
			warning("Failed to open file %s for output, using stdout instead\n",outfname);
			outfile = stdout;
			outfname = NULL;
		}
	}
	else
		outfile = stdout;
	normal("Ouput configured\n");

	// launch cmd
	status = sl->fn(argc,argv);
	if(status)
	{
		error("Command Failed\n");
		goto err;
	}

	if(fl->flags & NEED_OUTPUT)
	{
		normal("Printing graph\n");
		if(fl->flags & IS_GRAPH_P)
			status = ggen_write_graph(g_p,outfile);
		else
			status = ggen_write_graph(&g,outfile);

		if(status)
		{
			error("Writing graph failed\n");
			goto free_outg;
		}
		else
			normal("Graph printed\n");
	}
	if((fl->flags & NEED_RNG) && rngfname)
	{
		normal("Saving RNG state\n");
		status = ggen_rng_save(&rng,rngfname);
		if(status)
		{
			error("RNG saving failed\n");
		}
		else
			normal("RNG Saved\n");
	}
free_outg:
	if(outfname)
		fclose(outfile);

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
		return 1;
	}
	// check that user didn't ask for something crazy
	if(infname != NULL && !(c->flags & NEED_INPUT))
	{
		error("Input file not needed\n");
		return 1;
	}
	if(name != NULL && !(c->flags & NEED_NAME))
	{
		error("Property name not needed\n");
		return 1;
	}
	if(ptype != -1 && !(c->flags & NEED_TYPE))
	{
		error("Property type not needed\n");
		return 1;
	}
	if(rngfname != NULL && !(c->flags & NEED_RNG))
	{
		error("RNG state file not needed\n");
		return 1;
	}
	// find second lvl command
	info("Searching subcommand %s\n",argv[0]);
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
	error("Wrong subcommand\n");
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
			case 'r':
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
			case 'f':
				logfname = optarg;
				break;
			case 'l':
				logval = optarg;
				break;
			case ':':
				fprintf(stderr,"ggen: missing option argument at %s\n",argv[optind-1]);
				exit(EXIT_FAILURE);
			case '?':
				fprintf(stderr,"ggen: invalid option: %s\n",argv[optind-1]);
				exit(EXIT_FAILURE);
			default:
				fprintf(stderr,"ggen bug: someone forgot how to write a switch\n");
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
		exit(EXIT_FAILURE);
	}
	// initialize logging
	if(logfname != NULL)
	{
		logfile = fopen(logfname,"w");
		if(!logfile)
		{
			fprintf(stderr,"failed to open file %s for logging, using stderr instead\n",logfname);
			logfile = stderr;
			logfname = NULL;
		}
	}
	else
		logfile = stderr;
	status = log_init(logfile,"ggen");
	if(status)
	{
		fprintf(stderr,"error during log initialization\n");
		exit(EXIT_FAILURE);
	}
	// now set level according to options
	unsigned long l;
	if(logval != NULL)
	{
		status = s2ul(logval,&l);
		if(status)
		{
			warning("Cannot convert log level option to int\n");
			l = LOG_NORMAL;
		}
		else if(l < LOG_QUIET || l > LOG_DEBUG)
		{
			warning("Incorrect log level value, must be between %d and %d\n",LOG_QUIET,LOG_DEBUG);
			l = LOG_NORMAL;
		}
	}
	else
		l = LOG_NORMAL;
	log_filter_above((enum log_level)l);
	normal("Logging facility initialized\n");

	// initialize igraph attributes for all commands
	igraph_i_set_attribute_table(&igraph_cattribute_table);

	// find the command to launch
	info("Searching for command %s\n",argv[0]);
	for(int i = 0; i < ARRAY_SIZE(cmd_table); i++)
	{
		struct first_lvl_cmd *c = cmd_table+i;
		if(!strcmp(c->name,argv[0]))
		{
			argc--;
			argv++;
			status = handle_first_lvl(argc,argv,c);
			goto end;
		}
	}
	status = EXIT_FAILURE;
	error("Command not found\n");
end:
	// close logging
	normal("Closing log\n");
	if(logfname)
	{
		fclose(logfile);
	}
	return status;
}
