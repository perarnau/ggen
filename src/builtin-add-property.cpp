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
#include <getopt.h>
#include <inttypes.h>
#include <stdio.h>

#include <boost/config.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/lexical_cast.hpp>
#include <exception>
#include <iostream>
#include <fstream>

#include "types.hpp"
#include "random.hpp"
#include "dynamic_properties.hpp"
#include "graph-properties.hpp"
#include "builtin.hpp"

using namespace boost;
using namespace ggen;

static int do_dag = 0;
static int verbose = 0;
static int ask_help = 0;
static int ask_full_help = 0;

static int is_edge = 0, is_vertex = 0;

static Graph *g = NULL;
static ggen_rng *rng = NULL;
static ggen_rnd *rnd = NULL;
static dynamic_properties* properties = NULL;
static const char* name = "new_property";

static char* infile = NULL;
static std::ifstream fin;
static std::istream *in = &std::cin;

static char* outfile = NULL;
static std::ofstream fout;
static std::ostream *out = &std::cout;

static const char* general_help[] = {
	"Usage: ggen add-property [options] cmd args\n\n",
	"Generic options:\n",
	"--help                   : ask for help. When a method is provided only display help for this method\n",
	"--full-help              : display the full help message, including a detailled description of each command\n",
	"--verbose                : increase verbosity\n",
	"--input     <filename>   : specify an input file for the graph\n",
	"--output    <filename>   : specify a file for saved the graph\n",
	"\nProperty options:\n",
	"--name      <string>     : the name of the property\n",
	"--edge,--vertex          : force the type of property to add\n",
	"\nRandom Numbers options:\n",
	"--seed      <uint64>     : specify the generator seed\n",
	"--rng-file  <filename>   : load and save the generator state in a specific file\n", 
	"--rng-type  <uint>       : specify the generator type\n",
	"\nCommands available:\n",
	"gaussian                 : add a property following a gaussian distribution\n",
	"flat                     : add a property following a flat (uniform) distribution\n",
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

static struct help_elt helps[] = {
	{ "general" , general_help },
	{ "gaussian", gaussian_help },
	{ "flat", flat_help },
};

static int cmd_help(int argc, char** argv)
{
	usage_helps(argc,argv,helps,ask_full_help);
}

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


static void postop()
{
	if(is_edge)
		add_edge_property(g,properties,rnd,name);
	else
		add_vertex_property(g,properties,rnd,name);


	if(outfile)
	{
		fout.open(outfile);
		out = &fout;
	}

	write_graphviz(*out,*g,*properties);

	if(outfile)
		fout.close();
}

static int cmd_gaussian(int argc, char** argv)
{
	if(argc == 1)
		usage(gaussian_help);
	
	// get arguments
	if(argc < 2)
		die("wrong number of arguments");

	double sigma;

	try {
		sigma = lexical_cast<double>(argv[1]);
	}
	catch(std::exception &e)
	{
		die("bad arguments");
	}
	preop();
	rnd = new ggen_rnd_gaussian(rng,sigma);
	postop();
	return 0;
}

static int cmd_flat(int argc, char** argv)
{
	if(argc == 1)
		usage(flat_help);
	
	// get arguments
	if(argc < 3)
		die("wrong number of arguments");

	double min,max;	

	try {
		min = lexical_cast<double>(argv[1]);
		max = lexical_cast<double>(argv[2]);
	}
	catch(std::exception &e)
	{
		die("bad arguments");
	}
	preop();	
	rnd = new ggen_rnd_flat(rng,min,max);
	postop();
	return 0;
}

/* Commands to handle */
static struct cmd_table_elt cmd_table[] = {
	{ "help", cmd_help },
	{ "gaussian", cmd_gaussian },
	{ "flat", cmd_flat },
};


/* all command line arguments */
static struct option long_options[] = {
	/* general options */
	{ "verbose", no_argument, &verbose, 1 },
	{ "help", no_argument, &ask_help, 1 },
	{ "full-help", no_argument, &ask_full_help, 1 },
	{ "input", required_argument, NULL, 'i' },
	{ "output", required_argument, NULL, 'o' },
	/* property options */
	{ "name", required_argument, NULL, 'n' },
	{ "edge", no_argument, &is_edge, 1 },
	{ "vertex", no_argument, &is_vertex, 1 },
	/* random number generator */
	{ "seed", required_argument, NULL, 's' },
	{ "rng-type", required_argument, NULL, 't' },
	{ "rng-file", required_argument, NULL, 'f' },
	{ 0, 0, 0, 0},
};

static const char* short_opts = "hi:o:n:s:t:f:";

/** 
* Main program
*
*/
int cmd_add_property(int argc,char** argv)
{
	const char* cmd;
	g = NULL;

	int c,err;
	int option_index = 0;
	uint64_t seed;
	unsigned int type;
	char *file = NULL;
	bool seeded = false,filed = false,typed = false;
	// parse other options
	while(1)
	{
		c = getopt_long(argc, argv, short_opts,long_options, &option_index);	
		if(c == -1)
			break;

		switch(c)
		{
			case 'h':
			case 0:
				break;
			case 's':
				seed = strtoumax(optarg,NULL,10);
				seeded = true;
				break;
			case 't':
				err = sscanf(optarg,"%u",&type);
				if(!err)
					exit(1);
				typed = true;
				break;
			case 'f':
				file = optarg;
				filed = true;
				break;
			case 'o':
				outfile = optarg;
				break;
			case 'i':
				infile = optarg;
				break;
			case 'n':
				name = optarg;
				break;
			default:
				die("someone forgot how to write switches");
			case '?':
				die("option parsing got mad");
		}
	}

	// init rng
	rng = new ggen_rng();
	if(!seeded)
		seed = time(NULL);
	if(!typed)
		type = GGEN_RNG_DEFAULT;

	rng->allocate(type);
	rng->seed(seed);
	
	// this must be the last, as the rng must have been created
	if(filed)
	{
		rng->set_file(file);
		rng->read();
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
	if(!cmd)
		cmd = "help";

	int status = 0;
	// launch command
	if(!strcmp("help",cmd))
	{	
		status = cmd_help(argc,argv);
	}

	g =  new Graph();
	properties = new dynamic_properties(&create_property_map);

	for(int i = 1; i < ARRAY_SIZE(cmd_table); i++)
	{
		struct cmd_table_elt *c = cmd_table+i;
		if(!strcmp(c->name,cmd))
		{
			status = c->fn(argc,argv);
			delete g;
			delete properties;
			return 0;
		}
	}
	die("Wrong command !");
}
