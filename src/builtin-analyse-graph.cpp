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
#include <fstream>
#include <climits>
#include <getopt.h>

#include <boost/config.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/properties.hpp>

#include "types.hpp"
#include "builtin.hpp"
#include "graph-analysis.hpp"
#include "dynamic_properties.hpp"
#include "results.hpp"

using namespace boost;
using namespace ggen;

static int ask_help = 0;
static int verbose = 0;
static char* infile = NULL;
static char* result_format = NULL;

static Graph *g = NULL;
static dynamic_properties* properties;
static std::istream *in = &std::cin;

/* Result format handling */
static ggen_result_graph* create_result_graph()
{
	if(result_format == NULL || !strcmp(result_format,"stupid"))
	{
		return new ggen_rg_stupid();
	}
	else
		die("Unknown result format");
}

static ggen_result_paths* create_result_paths()
{
	if(result_format == NULL || !strcmp(result_format,"stupid"))
	{
		return new ggen_rp_stupid();
	}
	else
		die("Unknown result format");
}

static ggen_result_vmap* create_result_vmap()
{
	if(result_format == NULL || !strcmp(result_format,"stupid"))
	{
		return new ggen_rvm_stupid();
	}
	else
		die("Unknown result format");
}

/* usage and cmd functions */
static const char* general_help[] = {
	"Usage: ggen analyse-graph [options] cmd\n",
	"Allowed options:\n",
	"--help               : ask for help\n",
	"--verbose            : increase verbosity\n",
	"--input <filename>   : read the graph from a file\n",
	"--result-format      : change the way results are presented\n",
	"\nAllowed commands:\n",
	"nb-vertices          : gives the number of vertices in the graph\n",
	"nb-edges             : gives the number of edges in the graph\n",
	"mst                  : compute the Minimum Spanning Tree of the graph\n",
	"lp                   : compute the Longest Path of the graph\n",
	"npl                  : compute the Nodes Per Layer of the graph\n",
	"out-degree           : gives the out_degree of each vertex\n",
	"in-degree            : gives the in_degree of each vertex\n",
	"max-independent-set  : gives a maximum independent set of the graph\n",
	"strong-components    : gives the list of all strong components of the graph\n",
	"maximal-paths        : gives the list of all maximal paths (ending by a sink)\n",
	"\nResult Formats:\n",
	"Result formats make possible some additional computation during the analysis\n",
	"As of today, only <stupid> is defined.\n",
	NULL,
};

static int cmd_help(int argc, char **argv)
{
	usage(general_help);
}

static int cmd_nb_vertices(int argc, char **argv)
{
	std::cout << "Number of vertices: " << num_vertices(*g) << std::endl;
	return 0;
}

static int cmd_nb_edges(int argc, char **argv)
{
	std::cout << "Number of edges: " << num_edges(*g) << std::endl;
	return 0;
}

static int cmd_mst(int argc, char **argv)
{
	ggen_result_graph *r = create_result_graph();
	r->set_stream(&std::cout);
	minimum_spanning_tree(r,*g,*properties);
	delete r;
	return 0;
}

static int cmd_lp(int argc, char **argv)
{
	ggen_result_paths *r = create_result_paths();
	r->set_stream(&std::cout);
	longest_path(r,*g,*properties);
	delete r;
	return 0;
}

static int cmd_npl(int argc, char **argv)
{
	nodes_per_layer(*g,*properties);
	return 0;
}

static int cmd_out_degree(int argc, char **argv)
{
	ggen_result_vmap *r = create_result_vmap();
	r->set_stream(&std::cout);
	out_degree(r,*g,*properties);
	delete r;
	return 0;
}
static int cmd_in_degree(int argc, char **argv)
{
	ggen_result_vmap *r = create_result_vmap();
	r->set_stream(&std::cout);
	in_degree(r,*g,*properties);
	delete r;
}
static int cmd_max_indep_set(int argc, char **argv)
{
	max_independent_set(*g,*properties);
	return 0;
}
static int cmd_strong_components(int argc, char **argv)
{
	strong_components(*g,*properties);
	return 0;
}

static int cmd_maximal_paths(int argc, char **argv)
{
	ggen_result_paths *r = create_result_paths();
	r->set_stream(&std::cout);
	maximal_paths(r,*g,*properties);
	delete r;
	return 0;
}

static cmd_table_elt cmd_table[] = {
	{ "help", cmd_help },
	{ "nb-vertices", cmd_nb_vertices },
	{ "nb-edges",   cmd_nb_edges },
        { "mst", cmd_mst },
        { "lp", cmd_lp },
        { "npl", cmd_npl },
        { "out-degree", cmd_out_degree },
        { "in-degree", cmd_in_degree },
        { "max-independent-set", cmd_max_indep_set },
        { "strong-components", cmd_strong_components },
	{ "maximal-paths", cmd_maximal_paths },
};

static struct option long_options[] = {
	{ "help", no_argument, &ask_help, 1 },
	{ "verbose", no_argument, &verbose, 1 },
	{ "input", required_argument, NULL, 'i' },
	{ "result-format", required_argument, NULL, 'f' },
	{ 0,0,0,0 },
};

static const char short_opts[] = "hvi:";

int cmd_analyse_graph(int argc,char** argv)
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
			case 'f':
				result_format = optarg;
				break;
			case '?':
			default:
				exit(1);
		}
	}

	std::ifstream fin;
	if(infile)
	{
		fin.open(infile);
		in = &fin;
	}


	// now forget the parsed part of argv
	argc -= optind;
	argv = &(argv[optind]);

	// this is the command
	cmd = argv[0];
	if(!cmd)
		cmd = "help";

	int status = 0;
	if(!strcmp(cmd,"help"))
	{
		usage(general_help);
	}

	g = new Graph();
	properties = new dynamic_properties(&create_property_map);

	for(int i = 1; i < ARRAY_SIZE(cmd_table); i++)
	{
		struct cmd_table_elt *c = cmd_table+i;
		if(!strcmp(c->name,cmd))
		{
			read_graphviz(*in, *g,*properties);
			status = c->fn(argc,argv);
			goto ret;
		}
	}
	die("wrong command");

	ret:
	if(infile)
		fin.close();

	delete g;
	delete properties;
	return 0;
}
