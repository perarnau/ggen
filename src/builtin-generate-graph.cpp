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

/* We use extensively the BOOST library for 
* handling output, program options and random generators
*/

#include <boost/config.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/lexical_cast.hpp>
#include <exception>
#include <iostream>
#include <fstream>

#include "types.hpp"
#include "random.hpp"
#include "dynamic_properties.hpp"
#include "graph-generation.hpp"
#include "builtin.hpp"

using namespace boost;
using namespace ggen;

static int do_dag = 0;
static int verbose = 0;
static int ask_help = 0;
static int ask_full_help = 0;
static char *output = NULL;

static Graph *g = NULL;
static generation_context *cntxt = NULL;
static ggen_rng *rng = NULL;
static dynamic_properties properties(&create_property_map);
static std::ostream *out = &std::cout;


static const char* general_help[] = {
	"Usage: ggen generate-graph [options] method args\n\n",
	"Generic options:\n",
	"--help                   : ask for help. When a method is provided only display help for this method\n",
	"--full-help              : print the full help\n",
	"--verbose                : increase verbosity\n",
	"--output    <filename>   : specify a file for saved the graph\n",
	"\nGraph options:\n",
	"--do-dag                 : only generate a Directed Acyclic Graph if possible\n",
	"\nRandom Numbers options:\n",
	"--seed      <uint64>     : specify the generator seed\n",
	"--rng-file  <filename>   : load and save the generator state in a specific file\n", 
	"--rng-type  <uint>       : specify the generator type\n",
	"\nMethods available:\n",
	"erdos_gnp                : the classical adjacency matrix method\n",
	"erdos_gnm                : selection of edges in the complete graph\n",
	"layer                    : the classical layer by layer method\n",
	"random_orders            : generation of a DAG by intersection of total orders\n",
	"tgff                     : the method present in the paper of the same name\n",
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

static const char* layer_help[] = {
	"\nLayer By Layer:\n",
	"Split vertices into layers and connect layers between them.\n",
	"Arguments:\n",
	"     - number of vertices    : how many vertices in this graph\n",
	"     - number of layers      : how many layers to create\n",
	"     - probability           : chance to connect to vertices of different layers\n",
	NULL
};

static const char* random_help[] = {
	"\nRandom Orders:\n",
	"Build a partial order from the intersection of random total orders and transform it into a DAG.\n",
	"Arguments:\n",
	"     - number of vertices    : how many vertices in this graph\n",
	"     - number of total orders: how many total orders should we intersect\n",
	NULL
};

static const char* tgff_help[] = {
	"\nTGFF:\n",
	"Build a graph by a succession of fan-in and fan-out steps.\n",
	"Arguments:\n",
	"     - number of vertices    : lower bound on the number of vertices wanted\n",
	"     - max out degree        : the maximum out degree of each vertex\n",
	"     - max in degree         : the maximum in degree of each vertex\n",
	NULL
};

static void print_help(const char *message[]) {
	for(int i = 0; message[i] != NULL; i++)
		std::cout << message[i];
}

static int cmd_help(int argc, char** argv)
{
	print_help(general_help);
	if(ask_full_help)
	{
		std::cout << "\nDetailled help for each method:" << std::endl;
		print_help(gnp_help);
		print_help(gnm_help);
		print_help(layer_help);
		print_help(random_help);
		print_help(tgff_help);
	}
}

static void write_graph()
{
	//since we created the graph from scratch we need to add a property for the vertices
	std::string name("node_id");
	vertex_std_map *m = new vertex_std_map();
	vertex_assoc_map *am = new vertex_assoc_map(*m);
	properties.property(name,*am);

	int i = 0;
	std::pair<Vertex_iter,Vertex_iter> vp;
	for(vp = vertices(*g); vp.first != vp.second; vp.first++)
		put(name,properties,*vp.first,boost::lexical_cast<std::string>(i++));

	// Write graph
	////////////////////////////////////	
	write_graphviz(*out, *g,properties);
}

static int cmd_erdos_gnp(int argc, char** argv)
{
	int status = 0, err = 0;
	if(ask_help)
		goto help;	
	
	// get arguments
	if(argc < 3)
	{
		status = 1;
		goto help;
	}

	vertices_size number;
	double prob;

	try {
		number = lexical_cast<vertices_size>(argv[1]);
		prob = lexical_cast<double>(argv[2]);
	}
	catch(exception &e)
	{
		status = 1;
		goto help;
	}

	g = generation::erdos_gnp(*cntxt,number,prob,do_dag ? true : false);
	write_graph();
	goto ret;

	help:
	print_help(gnp_help);
	
	ret:	
	return status;
}

static int cmd_erdos_gnm(int argc, char** argv)
{
	int status = 0, err = 0;
	if(ask_help)
		goto help;	
	
	// get arguments
	if(argc < 3)
	{
		status = 1;
		goto help;
	}

	vertices_size number;
	edges_size edges;

	try {
		number = lexical_cast<vertices_size>(argv[1]);
		edges = lexical_cast<edges_size>(argv[2]);
	}
	catch(exception &e)
	{
		status = 1;
		goto help;
	}

	g = generation::erdos_gnm(*cntxt,number,edges,do_dag ? true : false);
	write_graph();
	goto ret;

	help:
	print_help(gnm_help);
	
	ret:	
	return status;
}

static int cmd_layer(int argc, char** argv)
{
	int status = 0, err = 0;
	vertices_size number;
	vertices_size layers;
	double prob;
	std::vector<int> layers_v;

	if(ask_help)
		goto help;	
	
	// get arguments
	if(argc < 4)
	{
		status = 1;
		goto help;
	}

	
	try {
		number = lexical_cast<vertices_size>(argv[1]);
		layers = lexical_cast<vertices_size>(argv[2]);
		prob = lexical_cast<double>(argv[3]);
	}
	catch(exception &e)
	{
		status = 1;
		goto help;
	}

	layers_v = generation::layer_allocation(*cntxt,layers,number);
	g = generation::layer_by_layer(*cntxt,number,prob,do_dag ? true : false,layers_v);
	write_graph();
	goto ret;

	help:
	print_help(layer_help);
	
	ret:	
	return status;

}

static int cmd_random_orders(int argc, char** argv)
{
	int status = 0, err = 0;
	if(ask_help)
		goto help;	
	
	// get arguments
	if(argc < 3)
	{
		status = 1;
		goto help;
	}

	vertices_size number;
	int posets;

	try {
		number = lexical_cast<vertices_size>(argv[1]);
		posets = lexical_cast<int>(argv[2]);
	}
	catch(exception &e)
	{
		status = 1;
		goto help;
	}

	g = generation::random_orders(*cntxt,number,posets);
	write_graph();
	goto ret;

	help:
	print_help(random_help);
	
	ret:	
	return status;
}

static int cmd_tgff(int argc, char** argv)
{
	int status = 0, err = 0;
	if(ask_help)
		goto help;	
	
	// get arguments
	if(argc < 4)
	{
		status = 1;
		goto help;
	}

	vertices_size number;
	int max_od,max_id;	

	try {
		number = lexical_cast<vertices_size>(argv[1]);
		max_od = lexical_cast<int>(argv[2]);
		max_id = lexical_cast<int>(argv[3]);
	}
	catch(exception &e)
	{
		status = 1;
		goto help;
	}

	g = generation::tgff(*cntxt,number,max_od,max_id);
	write_graph();
	goto ret;

	help:
	print_help(tgff_help);
	
	ret:	
	return status;
}

/* Commands to handle */
static struct cmd_table_elt cmd_table[] = {
	{ "help", cmd_help },
	{ "erdos_gnp", cmd_erdos_gnp },
	{ "erdos_gnm", cmd_erdos_gnm },
	{ "layer", cmd_layer },
	{ "random_orders", cmd_random_orders },
	{ "tgff" , cmd_tgff },
};


/* all command line arguments */
static struct option long_options[] = {
	/* general options */
	{ "verbose", no_argument, &verbose, 1 },
	{ "help", no_argument, &ask_help, 1 },
	{ "full-help", no_argument, &ask_full_help, 1 },
	{ "output", required_argument, NULL, 'o' },
	/* graph options */
	{ "do-dag", no_argument, &do_dag, 1 },
	/* random number generator */
	{ "seed", required_argument, NULL, 's' },
	{ "rng-type", required_argument, NULL, 't' },
	{ "rng-file", required_argument, NULL, 'f' },
	{ 0, 0, 0, 0},
};

static const char* short_opts = "vdho:s:t:f:";

////////////////////////////////////////////////////////////////////////////////
// MAIN
////////////////////////////////////////////////////////////////////////////////

/** 
* Main program
*
*/
int cmd_generate_graph(int argc,char** argv)
{
	const char* cmd;
	g = NULL;
	cntxt = new generation_context();

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
			case 'v':
			case 'd':
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
				output = optarg;
				break;
			case '?':
			default:
				exit(1);
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

	std::ofstream fout;
	if(output)
	{
		fout.open(output);
		out = &fout;
	}

	cntxt->set_rng(rng);
	
	// now forget the parsed part of argv
	argc -= optind;
	argv = &(argv[optind]);

	// this is the command
	cmd = argv[0];
	if(!cmd)
		cmd = "help";

	int status = 0;
	// launch command
	for(int i = 0; i < ARRAY_SIZE(cmd_table); i++)
	{
		struct cmd_table_elt *c = cmd_table+i;
		if(!strcmp(c->name,cmd))
		{
			status = c->fn(argc,argv);
			goto ret;
		}
	}
	// if you finish here the command was wrong
	std::cerr << "Wrong command !" << std::endl;
	cmd_help(argc,argv);
	return 1;
	
	ret:
	if(output)
		fout.close();

	delete cntxt;
	delete g;
	
	return 0;
}
