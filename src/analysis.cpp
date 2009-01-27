/* GGen is a random graph generator :
 * it provides means to generate a graph following a
 * collection of methods found in the litterature.
 *
 * This is a research project founded by the MOAIS Team,
 * INRIA, Grenoble Universities.
 */

#include <iostream>

/* We use extensively the BOOST library for 
 * handling output, program options and random generators
 */
#include <boost/config.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/properties.hpp>


#include "dynamic_properties.hpp"

using namespace boost;


/* This is the definition of the graph struture
 * According to boost that means :
 *	* The graph is an adjacency list
 *	* The vertices are managed as a std::Vector
 *	* The edges are managed as std::set to enforce no parallel edges
 *	* The graph is bidirectional
 *	* We don't have any additional properties on vertices or edges
 */
typedef adjacency_list<setS, vecS, bidirectionalS,
	dynamic_properties, dynamic_properties> Graph;

////////////////////////////////////////////////////////////////////////////////
// MAIN
////////////////////////////////////////////////////////////////////////////////

dynamic_properties properties;
Graph *g;


/* Main program
 */
int main(int argc, char** argv)
{
	int nb_vertices,nb_edges;


	// Graph generation
	////////////////////////////////
	
	g = new Graph();
	create_default_vertex_property(properties,*g);

	// Write graph
	////////////////////////////////////	
	read_graphviz(std::cin, *g,properties);

	write_graphviz(std::cout, *g,properties);
	delete g;
	return 0;
}
