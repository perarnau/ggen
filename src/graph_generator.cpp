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
#include <boost/program_options.hpp>
#include <boost/graph/graphviz.hpp>

#include "ggen.hpp"
#include "random.hpp"
#include "dynamic_properties.hpp"

using namespace boost;

////////////////////////////////////////////////////////////////////////////////
// Generation Methods
////////////////////////////////////////////////////////////////////////////////

/* DEVELOPPERS: 
 * all generation methods must use the same prototype :
 	
 	void gg_##method_name##(Graph& g, int num_vertices, int num_edges, base_generator_type& gen, bool allow_parallel = false, bool self_edges = false)
 
 * This prototype come from BOOST and seem to handle all the complexity a generation method can have.
 */



/* Random generation by the adjacency matrix method :
 * Run through the adjacency matrix
 * and at each i,j decide if matrix[i][j] is an edge by tossing a coin
 */
void gg_adjacency_matrix(Graph& g,int num_vertices,int num_edges, ggen_rnd& rnd, bool allow_parallel /*= false*/, bool self_edges /*= false*/);


/* Random generation by choosing pairs of vertices.
*/ 
void gg_random_vertex_pairs(Graph& g,int num_vertices, int num_edges, ggen_rnd& rnd, bool allow_parallel = false, bool self_edges = false) {	

	g = Graph(num_vertices);

	// create a two arrays for ggen_rnd::choose
	boost::any *src = new boost::any[num_vertices];
	boost::any *dest = new boost::any[2];
	
	// add all vertices to src
	int i = 0;
	std::pair<Vertex_iter, Vertex_iter> vp;
	for (vp = boost::vertices(g); vp.first != vp.second; ++vp.first)
		src[i++] = boost::any_cast<Vertex>(*vp.first);

	int added_edges = 0;
	while(added_edges < num_edges ) {
		if( !self_edges)
		{
			rnd.choose(dest,2,src,num_vertices,sizeof(boost::any));
		}
		else
		{
			// TODO rnd.shuffle
		}
		Vertex u,v;
		u = boost::any_cast<Vertex>(dest[0]);
		v = boost::any_cast<Vertex>(dest[1]);
		std::pair<Edge,bool> result = add_edge(u,v,g);
		if(!allow_parallel && result.second)
			added_edges++;
	}

	//delete src;
	//delete dest;
}

////////////////////////////////////////////////////////////////////////////////
// MAIN
////////////////////////////////////////////////////////////////////////////////

namespace po = boost::program_options;
dynamic_properties properties(&create_property_map);
Graph *g;

/* Main program
 */
int main(int argc, char** argv)
{
	int nb_vertices,nb_edges;

	// Handle command line arguments
	////////////////////////////////////
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("nb-vertices,n", po::value<int>(&nb_vertices)->default_value(10),"set the number of vertices in the generated graph")
		("nb-edges,m", po::value<int>(&nb_edges)->default_value(10),"set the number of edges in the generated graph")
	;

	po::options_description all;
	po::options_description ro = random_add_options();


	all.add(desc).add(ro);


	po::variables_map vm;
	po::store(po::parse_command_line(argc,argv,all),vm);
	po::notify(vm);
	
	if (vm.count("help")) {
		std::cout << desc << "\n";
		        return 1;
	}

	random_options_state rs;
	random_options_start(vm,rs);
	
	// Graph generation
	////////////////////////////////
	
	g = new Graph();

	gg_random_vertex_pairs(*g,nb_vertices,nb_edges,*rs.rnd);
	
	// since we created the graph from scratch we need to do add a property for the vertices
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
	write_graphviz(std::cout, *g,properties);
	delete g;
	return 0;
}
