/* GGen is a random graph generator :
 * it provides means to generate a graph following a
 * collection of methods found in the litterature.
 *
 * This is a research project founded by the MOAIS Team,
 * INRIA, Grenoble Universities.
 */

#ifndef GRAPH_GENERATOR_H
#define GRAPH_GENERATOR_H

/* We use extensively the BOOST library for 
 * handling output, program options and random generators
 */
#include <boost/config.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/random.hpp>
#include <boost/graph/properties.hpp>
#include <boost/dynamic_property_map.hpp>

#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/program_options.hpp>


using namespace boost;

/* Random generation stuff
 *  we define a base generator and each graph generation method
 *  use this base generator to define the random generator it wants
 */

typedef minstd_rand base_generator_type;

////////////////////////////////////////////////////////////////////////////////
// Generation Methods
////////////////////////////////////////////////////////////////////////////////

/* DEVELOPPERS: 
 * all generation methods must use the same prototype :
 	
 	void gg_##method_name##(Graph& g, int num_vertices, int num_edges, base_generator_type& gen, bool allow_parallel = false, bool self_edges = false)
 
 * This prototype come from BOOST and seem to handle all the complexity a generation method can have.
 */



/* Boost graph generator,
 * Any hint on what is exactly done is welcome
 * Swann : as of today the boost code seems to 
 * make a variant of our random_vertex_pairs method.
 */
void gg_boost(Graph& g,int num_vertices, int num_edges, base_generator_type& gen, bool allow_parallel = false, bool self_edges = false);

/* Random generation by the adjacency matrix method :
 * Run through the adjacency matrix
 * and at each i,j decide if matrix[i][j] is an edge by tossing a coin
 */
void gg_adjacency_matrix(Graph& g,int num_vertices,int num_edges, base_generator_type& gen, bool allow_parallel = false, bool self_edges = false);


/* Random generation by choosing pairs of vertices.
*/ 
void gg_random_vertex_pairs(Graph& g,int num_vertices, int num_edges, base_generator_type& gen, bool allow_parallel = false, bool self_edges = false);


////////////////////////////////////////////////////////////////////////////////
// Command line parsing
////////////////////////////////////////////////////////////////////////////////

void parse_distribution(std::string name,std::string s);

void parse_property(std::string s);

#endif
