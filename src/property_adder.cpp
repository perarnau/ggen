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

/* We use extensively the BOOST library for 
 * handling output, program options and random generators
 */
#include <boost/config.hpp>
#include <boost/program_options.hpp>
#include <boost/regex.hpp>
#include <boost/graph/graphviz.hpp>

#include "ggen.hpp"
#include "dynamic_properties.hpp"
#include "random.hpp"

using namespace boost;
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Global Definitions
////////////////////////////////////////////////////////////////////////////////

dynamic_properties properties(&create_property_map);
Graph *g;

ggen_rng* global_rng;
ggen_rnd* global_rnd;

////////////////////////////////////////////////////////////////////////////////
// MAIN
////////////////////////////////////////////////////////////////////////////////

namespace po = boost::program_options;

void add_vertex_property(string name)
{
	vertex_std_map* map = new vertex_std_map();
	vertex_assoc_map * amap = new vertex_assoc_map(*map);
	properties.property(name,*amap);
		
	// iterate and add random property
	std::pair<Vertex_iter, Vertex_iter> vp;
	for (vp = boost::vertices(*g); vp.first != vp.second; ++vp.first)
		    put(name,properties,*vp.first,boost::lexical_cast<std::string>(global_rnd->get()));
}

void add_edge_property(string name)
{
	edge_std_map* map = new edge_std_map();
	edge_assoc_map * amap = new edge_assoc_map(*map);
	properties.property(name,*amap);

	// iterate and add random property
	std::pair<Edge_iter, Edge_iter> ep;
	for (ep = boost::edges(*g); ep.first != ep.second; ++ep.first)
		    put(name,properties,*ep.first,boost::lexical_cast<std::string>(global_rnd->get()));
	
}


/* Main program
 */
int main(int argc, char** argv)
{
	int nb_vertices,nb_edges;
	string name;
	bool edge_property;

	// Handle command line arguments
	////////////////////////////////////
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		
		/* I/O options */
		("input,i", po::value<string>(), "Set the input file")
		("output,o", po::value<string>(), "Set the output file")

		/* Property options */
		("name,n",po::value<string>(),"Set the property name")
		("edge,e",po::value<bool>()->zero_tokens(),"Add an edge property instead of a vertex one")
	;

	po::options_description rngo = random_rng_options();
	po::options_description rndo = random_rnd_options();
	po::options_description all;
	all.add(desc).add(rngo).add(rndo);
	
	
	// Parse command line options
	////////////////////////////////
	po::variables_map vm;
	po::store(po::parse_command_line(argc,argv,all),vm);
	po::notify(vm);
	
	if (vm.count("help"))
	{
		std::cout << all << "\n";
		        return 1;
	}

	if (vm.count("input")) 
	{
		// open the file for reading
		int in = open(vm["input"].as<std::string>().c_str(),O_RDONLY);
	
		// redirect stdout to it
		dup2(in,STDIN_FILENO);
		close(in);
	}
	
	if (vm.count("output")) 
	{
		// create a new file with 344 file permissions
		int out = open(vm["output"].as<std::string>().c_str(),O_WRONLY | O_CREAT | O_TRUNC,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );
	
		// redirect stdout to it
		dup2(out,STDOUT_FILENO);
		close(out);
	}

	if(vm.count("name"))
	{
		name = vm["name"].as<string>();
	}
	else
		name = "NewProperty";

	if(vm.count("edge"))
	{
		edge_property = true;
	}
	else
		edge_property = false;


	global_rng = random_rng_handle_options_atinit(vm);
	global_rnd = random_rnd_handle_options_atinit(vm,global_rng);

	// Graph generation
	////////////////////////////////
	
	g = new Graph();

	// Read graph
	////////////////////////////////////	
	read_graphviz(std::cin, *g,properties);

	
	// Add property
	////////////////////////////////////
	if(edge_property)
		add_edge_property(name);
	else
		add_vertex_property(name);
	
	// Write graph
	////////////////////////////////////	
	write_graphviz(std::cout, *g,properties);
	
	random_rnd_handle_options_atexit(vm,global_rng,global_rnd);
	random_rng_handle_options_atexit(vm,global_rng);

	delete g;
	return 0;
}
