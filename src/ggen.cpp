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
#include <boost/regex.hpp>
#include <boost/graph/graphviz.hpp>

#include "ggen.hpp"
#include "dynamic_properties.hpp"

using namespace boost;

uint64_t random_seed;
base_generator_type* random_generator;

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
void gg_boost(Graph& g,int num_vertices, int num_edges, base_generator_type& gen, bool allow_parallel /*= false*/, bool self_edges /*= false*/) {

	boost::generate_random_graph(g,num_vertices,num_edges,gen,allow_parallel,self_edges);
}

/* Random generation by the adjacency matrix method :
 * Run through the adjacency matrix
 * and at each i,j decide if matrix[i][j] is an edge by tossing a coin
 */
void gg_adjacency_matrix(Graph& g,int num_vertices,int num_edges, base_generator_type& gen, bool allow_parallel /*= false*/, bool self_edges /*= false*/);


/* Random generation by choosing pairs of vertices.
*/ 
void gg_random_vertex_pairs(Graph& g,int num_vertices, int num_edges, base_generator_type& gen, bool allow_parallel /*= false*/, bool self_edges /*= false*/) {	

	g = Graph(num_vertices);

	/* We want a random generator with an uniform distribution over 0,num_vertices
	*/
	boost::uniform_int<> uni_dist(0,num_vertices-1);
	boost::variate_generator<base_generator_type&, boost::uniform_int<> > uni(gen,uni_dist);

	int added_edges = 0;
	while(added_edges < num_edges ) {
		int u =  uni();
		int v =  uni();
		if( !self_edges && u == v )
			continue;

		std::pair<graph_traits<Graph>::edge_descriptor,bool> result = add_edge(u,v,g);
		if(!allow_parallel && result.second)
			added_edges++;
	}
}

////////////////////////////////////////////////////////////////////////////////
// MAIN
////////////////////////////////////////////////////////////////////////////////

namespace po = boost::program_options;
dynamic_properties properties;
Graph *g;

void parse_distribution(std::string name,std::string s)
{	
	// for now we only know of the uniform distribution of integers
	static const boost::regex uni_int("uni_int\\((\\d+),(\\d+)\\)");
	boost::smatch what;
	if(boost::regex_match(s,what,uni_int))
	{
		int min,max;
		min = boost::lexical_cast<int>(what[1]);
		max =  boost::lexical_cast<int>(what[2]);
		boost::uniform_int<> uni_dist(min,max);
		boost::variate_generator<base_generator_type&, boost::uniform_int<> > uni(*random_generator,uni_dist);
		
		typedef std::map< graph_traits<Graph>::vertex_descriptor, int > user_map;
		typedef graph_traits<Graph>::vertex_iterator vertex_iter;
		typedef boost::associative_property_map< user_map > vertex_map;

		user_map* map = new user_map();
		vertex_map * bmap = new vertex_map(*map);
		properties.property(name,*bmap);
		
		std::pair<vertex_iter, vertex_iter> vp;
		for (vp = boost::vertices(*g); vp.first != vp.second; ++vp.first)
			    put(name,properties,*vp.first,uni());
		
	}
}

void parse_property(std::string s)
{
	int i = s.find(':');
	if(i!= std::string::npos) 
	{
		std::string property_name = s.substr(0,i);
		std::string distribution_definition = s.substr(i+1);
		parse_distribution(property_name,distribution_definition);
	}
}


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
		("seed,s", po::value<uint64_t>(&random_seed)->default_value(time(NULL)), "set the random generator seed")
		("nb-vertices,n", po::value<int>(&nb_vertices)->default_value(10),"set the number of vertices in the generated graph")
		("nb-edges,m", po::value<int>(&nb_edges)->default_value(10),"set the number of edges in the generated graph")
		("vertex-property,v",po::value< std::vector<std::string> >(), "vertex properties to generate")
	;
	
	po::variables_map vm;
	po::store(po::parse_command_line(argc,argv,desc),vm);
	po::notify(vm);
	
	if (vm.count("help")) {
		std::cout << desc << "\n";
		        return 1;
	}

	
	// Create used random_generator
	////////////////////////////////
	random_generator = new base_generator_type(random_seed);


	// Graph generation
	////////////////////////////////
	
	g = new Graph();

	//g = generate_graph(num_vertices,num_edges);
	//write_graphviz(std::cout, *g);
	//delete g;

	
	gg_random_vertex_pairs(*g,nb_vertices,nb_edges,*random_generator);
	

	// Handle parsing and generation of graph properties
	///////////////////////////////////////////////////////
	create_default_vertex_property(properties,*g);
	//properties.property("node_id",get(vertex_name,*g));
	
	if (vm.count("vertex-property")) {

		std::vector < std::string > v = vm["vertex-property"].as< std::vector< std::string> >();
		for (std::vector<std::string>::iterator it = v.begin(); it != v.end();it++)
		{
			parse_property(*it);
		}
	}

	// Write graph
	////////////////////////////////////	
	write_graphviz(std::cout, *g,properties);
	delete g;
	return 0;
}
