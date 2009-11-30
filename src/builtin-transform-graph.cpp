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

#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <climits>
#include <getopt.h>

/* We use extensively the BOOST library for 
 * handling output, program options and random generators
 */
#include <boost/config.hpp>

#include <boost/graph/graphviz.hpp>
#include <boost/graph/properties.hpp>

#include "types.hpp"
#include "builtin.hpp"
#include "graph-transformation.hpp"
#include "dynamic_properties.hpp"

using namespace boost;
using namespace std;
using namespace ggen;


static int ask_help = 0;
static int verbose = 0;
static char* infile = NULL;
static char* outfile = NULL;
static std::ostream *out = &std::cout;
static std::istream *in = &std::cin;

static Graph *g = NULL;
static dynamic_properties *properties = NULL;

static const char* general_help[] = {
	"Usage: ggen transform-graph [options] cmd\n",
	"Allowed options:\n",
	"--help                  : ask for help\n",
	"--verbose               : increase verbosity\n",
	"--input     <filename>  : read input from a given file\n",
	"--ouput     <filename>  : use a given file as output\n",
	"\nAllowed commands:\n",
	"remove-sinks            : remove all sinks present in the graph\n",
	"remove-sources          : remove all sources present in the graph\n",
	"add-sink    <name>      : add a node connected to all previous sinks\n",
	"add-source  <name>      : add a node connected to all previous sources\n",
	NULL,
};

static int cmd_help(int argc, char** argv)
{
	usage(general_help);
}

// pre processing operations
static std::ifstream fin;
static void preop()
{
	if(infile)
	{
		fin.open(infile);
		in = &fin;
	}
	read_graphviz(*in,*g,*properties);

	if(infile)
		fin.close();
}

// post processing operations
static std::ofstream fout;
static void postop()
{
	if(outfile)
	{
		fout.open(outfile);
		out = &fout;
	}

	write_graphviz(*out,*g,*properties);

	if(outfile)
		fout.close();
}

static int cmd_remove_sinks(int argc, char** argv)
{
	remove_sinks(g);
	return 0;
}

static int cmd_remove_sources(int argc, char** argv)
{
	remove_sources(g);
	return 0;
}

static int cmd_add_sink(int argc, char** argv)
{
	if(argc < 2)
		return 1;
	
	char *name = argv[1];
	std::string n(name);
	add_dummy_sink(g,properties,n);
	return 0;
}

static int cmd_add_source(int argc, char** argv)
{
	if(argc < 2)
		return 1;
	
	char *name = argv[1];
	std::string n(name);
	add_dummy_source(g,properties,n);
	return 0;
}

static cmd_table_elt cmd_table[] = {
	{ "help", cmd_help },
	{ "remove-sinks", cmd_remove_sinks },
	{ "remove-sources", cmd_remove_sources },
	{ "add-sink", cmd_add_sink },
	{ "add-source", cmd_add_source },
};

static struct option long_options[] = {
	{ "help", no_argument, &ask_help, 1 },
	{ "verbose", no_argument, &verbose, 1 },
	{ "input", required_argument, NULL, 'i' },
	{ "output", required_argument, NULL, 'o'},
	{0, 0, 0, 0},
};

static const char* short_opts = "hvi:o:";

int cmd_transform_graph(int argc, char** argv)
{
	const char* cmd;
	int c,err;
	int option_index = 0;
	
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
			case 'i':
				infile = optarg;
				break;
			case 'o':
				outfile = optarg;
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
	if(!cmd)
		cmd = "help";

	int status = 0;
	// launch command, we skip help because
	// all the other command need pre and post processing
	if(!strcmp("help", cmd))
	{
		status = cmd_help(argc,argv);
	}
	
	g = new Graph();
	properties = new dynamic_properties(&create_property_map);

	for(int i = 1; i < ARRAY_SIZE(cmd_table); i++)
	{
		struct cmd_table_elt *c = cmd_table+i;
		if(!strcmp(c->name,cmd))
		{
			preop();
			status = c->fn(argc,argv);
			postop();
			goto ret;
		}
	}
	die("wrong command");

	ret:
	delete g;
	delete properties;
	return 0;
}
