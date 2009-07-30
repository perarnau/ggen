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
#include "graph-properties.hpp"
#include "dynamic_properties.hpp"

using namespace boost;
using namespace ggen;

static int ask_help = 0;
static int verbose = 0;
static char* infile = NULL;
static int is_edge = 0, is_vertex = 1;
static const char* name = "node_id";

static Graph *g = NULL;
static dynamic_properties* properties;
static std::istream *in = &std::cin;

static const char* general_help[] = {
	"Usage: ggen analyse-graph [options] name\n",
	"Allowed options:\n",
	"--help               : ask for help\n",
	"--verbose            : increase verbosity\n",
	"--input <filename>   : read the graph for a file\n",
	"--edge,--vertex      : force the type of the property to extract\n",
	"\nArguments:\n",
	"name          : the name of the property to extract\n",
	NULL
};

static int cmd_nb_vertices(int argc, char **argv)
{
	read_graphviz(*in, *g,*properties);
	std::cout << "Number of vertices: " << num_vertices(*g) << std::endl;
	return 0;
}

static struct option long_options[] = {
	{ "help", no_argument, &ask_help, 1 },
	{ "verbose", no_argument, &verbose, 1 },
	{ "input", required_argument, NULL, 'i' },
	{ "edge", no_argument, &is_edge, 1 },
	{ "vertex", no_argument, &is_vertex, 1 },
	{ 0,0,0,0 },
};

static const char short_opts[] = "hvi:";

int cmd_extract_property(int argc, char** argv)
{
	const char* cmd;
	int c,err;
	int option_index = 0;
	int status = 0;
	
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

	if(is_edge && is_vertex)
	{
		die("you cannot specify both --edge and --vertex");
	}
	
			
	// now forget the parsed part of argv
	argc -= optind;
	argv = &(argv[optind]);

	// this is the name
	if(argc == 1)
		name = argv[0];
	else
		die("you must %sprovide a name",argc > 1 ? "only ":"");


	g = new Graph();
	properties = new dynamic_properties(&create_property_map);
	read_graphviz(*in, *g, *properties);
	if(infile)
		fin.close();
	
	if(is_edge)
	{
		extract_edge_property(&std::cout,g,properties,name);
	}
	else
	{
		extract_vertex_property(&std::cout,g,properties,name);
	}
	
	delete g;
	delete properties;
	return status;
}
