#include <iostream>
#include <boost/config.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
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

  

  graph_traits < Graph>::edge_iterator ei, ei_end;
  int nb_vertices = atoi(argv[1]);
  int nb_edges = atoi(argv[2]);
  Graph *g;
  //g = generate_graph(num_vertices,num_edges);
  //write_graphviz(std::cout, *g);
  //delete g;
  
  g = generate_graph_random_vertex_pairs(nb_vertices,nb_edges);

  typedef graph_traits < Graph >::vertex_descriptor vertex_descriptor;
  typedef graph_traits < Graph >::edge_descriptor edge_descriptor;

  std::vector<vertex_descriptor> p(num_vertices(*g));
  std::vector<int> d(num_vertices(*g));
  vertex_descriptor s = vertex(1, *g);
  
  property_map< Graph, edge_index_t >::type weightmap = get(edge_index, *g);
  property_map< Graph, vertex_index_t>::type indexmap = get(vertex_index, *g);

  dijkstra_shortest_paths(*g, s, &p[0], &d[0], weightmap, indexmap, std::less<int>(), closed_plus<int>(), (std::numeric_limits<int>::max)(), 0, default_dijkstra_visitor());


  std::cout << "distances and parents:" << std::endl;
  graph_traits < Graph>::vertex_iterator vi, vend;
  for (tie(vi, vend) = vertices(*g); vi != vend; ++vi) {
	  std::cout << "distance(" << *vi << ") = " << d[*vi] << ", ";
	  std::cout << "parent(" << *vi << ") = " << p[*vi] << std::
		  endl;
  }
  std::cout << std::endl;

  std::ofstream dot_file("figs/dijkstra-eg.dot");

  dot_file << "digraph D {\n"
	  << "  rankdir=LR\n"
	  << "  size=\"4,3\"\n"
	  << "  ratio=\"fill\"\n"
	  << "  edge[style=\"bold\"]\n" << "  node[shape=\"circle\"]\n";

  for (tie(ei, ei_end) = edges(*g); ei != ei_end; ++ei) {
	  graph_traits < Graph>::edge_descriptor e = *ei;
	  graph_traits < Graph>::vertex_descriptor
		  u = source(e, *g), v = target(e, *g);
	  dot_file << u << " -> " << v
		  << "[label=\"" << e << "\"";
	  if (p[v] == u)
		  dot_file << ", color=\"black\"";
	  else
		  dot_file << ", color=\"grey\"";
	  dot_file << "]";
  }
  dot_file << "}";



  //write_graphviz(std::cout, *g);
  delete g;
  return 0;
}
