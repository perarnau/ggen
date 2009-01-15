#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/random.hpp>
#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_int.hpp>
using namespace boost;


/* Random generation stuff
 *  we define a base generator and each graph generation method
 *  use this base generator to define the random generator it wants
 */

typedef minstd_rand base_generator_type;



/* This is the definition of the graph struture
 * According to boost that means :
 *	* The graph is an adjacency list
 *	* The vertices are managed as a std::Vector
 *	* The edges are managed as std::set to enforce no parallel edges
 *	* The graph is bidirectional
 *	* We don't have any additional properties on vertices or edges
 */
typedef adjacency_list<setS, vecS, bidirectionalS,
		       no_property, no_property> Graph;

/* Boost graph generator,
 * Any hint on what is exactly done is welcome
 */
Graph *generate_graph(int num_vertices, int num_edges) {

  Graph *g = new Graph;
  rand48 r = rand48((uint64_t) 0);
  generate_random_graph(*g,num_vertices,num_edges,r,false,false);
  return g;
}

/* Random generation by the adjacency matrix method :
 * Run through the adjacency matrix
 * and at each i,j decide if matrix[i][j] is an edge by tossing a coin
 */




/* Random generation by choosing pairs of vertices.
 */ 
Graph *generate_graph_random_vertex_pairs(int num_vertices, int num_edges) {	
  
  Graph *g = new Graph(num_vertices);

  /* We want a random generator with an uniform distribution over 0,num_vertices
   */
  base_generator_type generator(42u);
  boost::uniform_int<> uni_dist(0,num_vertices-1);
  boost::variate_generator<base_generator_type&, boost::uniform_int<> > uni(generator,uni_dist);

  int added_edges = 0;
  while(added_edges < num_edges ) {
	int u =  uni();
	int v =  uni();
	if( u == v )
		continue;

	std::pair<graph_traits<Graph>::edge_descriptor,bool> result = add_edge(u,v,*g);
	if(result.second)
		added_edges++;
  }

  return g;
}


/* Main program
 * TODO : proper argument handling
 */
int main(int argc, char** argv)
{
  if(argc != 3) 
    return 0;

  

  int num_vertices = atoi(argv[1]);
  int num_edges = atoi(argv[2]);
  Graph *g;
  //g = generate_graph(num_vertices,num_edges);
  //write_graphviz(std::cout, *g);
  //delete g;
  
  g = generate_graph_random_vertex_pairs(num_vertices,num_edges);
  write_graphviz(std::cout, *g);
  delete g;
  return 0;
}
