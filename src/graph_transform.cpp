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

/* We use extensively the BOOST library for 
 * handling output, program options and random generators
 */
#include <boost/config.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/properties.hpp>
#include <boost/program_options.hpp>

#include "ggen.hpp"
#include "dynamic_properties.hpp"

using namespace boost;
using namespace std;

dynamic_properties properties(&create_property_map);

////////////////////////////////////////////////////////////////////////////////
// TRANFORM FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

// Remove the sources present in the graph when it is passed to the function.
// We must be carefull to not remove too many nodes
void remove_sources(Graph* g)
{
	// the list of real sources
	std::list < Vertex > sources;
	
	// identify the sources as they don't have any in_edge
	std::pair <Vertex_iter, Vertex_iter> vp;
	for(vp = vertices(*g);vp.first != vp.second; vp.first++)
	{
		std::pair < In_edge_iter, In_edge_iter > ep = in_edges(*vp.first,*g);
		if(ep.first == ep.second)
		{
			sources.push_back(*vp.first);
		}
	}
	
	// now that we identied all the sources, remove them
	std::list<Vertex>::iterator it;
	for(it = sources.begin(); it != sources.end(); it++)
	{
		clear_vertex(*it,*g);
		remove_vertex(*it,*g);
	}
}

// Remove the sinks present in the graph when it is passed to the function.
// We must be carefull to not remove too many nodes
void remove_sinks(Graph* g)
{
	// the list of real sources
	std::list < Vertex > sinks;
	
	// identify the sinks as they don't have any out_edge
	std::pair <Vertex_iter, Vertex_iter> vp;
	for(vp = vertices(*g);vp.first != vp.second; ++vp.first)
	{
		std::pair < Out_edge_iter, Out_edge_iter > ep = out_edges(*vp.first,*g);
		if(ep.first == ep.second)
		{
			sinks.push_back(*vp.first);
		}
	}
	
	// now that we identied all the sinks, remove them
	std::list<Vertex>::iterator it;
	for(it = sinks.begin(); it != sinks.end(); it++)
	{
		clear_vertex(*it,*g);
		remove_vertex(*it,*g);
	}
}

////////////////////////////////////////////////////////////////////////////////
// MAIN
////////////////////////////////////////////////////////////////////////////////

namespace po = boost::program_options;

Graph *g;


/* Main program
*/
int main(int argc, char** argv)
{

	// Handle command line arguments
	////////////////////////////////////
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce this help message")

		/* I/O options */
		("input,i", po::value<string>(), "Set the input file")
		("output,o", po::value<string>(), "Set the output file")
		
		/* Transform options */
		("remove-sinks", po::value<bool>()->zero_tokens(), "Remove all sinks from the graph")
		("remove-sources", po::value<bool>()->zero_tokens(), "Remove all sources from the graph")
		;
		
	po::options_description all;
	all.add(desc);


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
		int out = open(vm["output"].as<std::string>().c_str(),O_RDWR | O_CREAT,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );
	
		// redirect stdout to it
		dup2(out,STDOUT_FILENO);
		close(out);
	}

	// Graph init
	////////////////////////////////

	g = new Graph();

	// Read graph
	////////////////////////////////	
	read_graphviz(std::cin, *g,properties);

	// Transfrom graph
	////////////////////////////////

	// Actualy this might be hard in place :
	// we _MUST_ be sure that the transformation do not propagates to other nodes.

	if(vm.count("remove-sources"))
	{
		remove_sources(g);
	}
	
	if(vm.count("remove-sinks"))
	{
		remove_sinks(g);
	}

	// Output graph
	////////////////////////////////
	
	write_graphviz(std::cout, *g,properties);

	delete g;
	return EXIT_SUCCESS;
}
