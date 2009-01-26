#include <iostream>
#include <boost/config.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/random.hpp>
#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/program_options.hpp>
#include <boost/regex.hpp>
using namespace boost;


/* Random generation stuff
 *  we define a base generator and each graph generation method
 *  use this base generator to define the random generator it wants
 */

typedef minstd_rand base_generator_type;
uint64_t random_seed;
base_generator_type* random_generator;

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
	boost::uniform_int<> uni_dist(0,num_vertices-1);
	boost::variate_generator<base_generator_type&, boost::uniform_int<> > uni(*random_generator,uni_dist);

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

////////////////////////////////////////////////////////////////////////////////
// MAIN
////////////////////////////////////////////////////////////////////////////////

namespace po = boost::program_options;

std::map< std::string, void*> generator_map;
boost::dynamic_properties vertex_p;

void* parse_distribution(std::string s)
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
		boost::variate_generator<base_generator_type&, boost::uniform_int<> >* uni = new boost::variate_generator<base_generator_type&, boost::uniform_int<> >(*random_generator,uni_dist);
		return uni;
	}
	else
		return NULL;
}

std::pair<std::string,void *>* parse_property(std::string s)
{
	std::pair<std::string,void*> *result = NULL;
	int i = s.find(':');
	if(i!= std::string::npos) 
	{
		std::string property_name = s.substr(0,i);
		std::string distribution_definition = s.substr(i+1);
		void *gen = parse_distribution(distribution_definition);
		if(gen != NULL)
		{
			result = new std::pair<std::string,void*>(property_name,gen);
			return result;
		}
		else
			return NULL;
	}
	else
		return NULL; 
}


/* Main program
 */
int main(int argc, char** argv)
{
	int nb_vertices,nb_edges;
	generator_map();
	vertex_p();


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

	if (vm.count("vertex-property")) {
		std::vector < std::string > v = vm["vertex-property"].as< std::vector< std::string> >();
		for (std::vector<std::string>::iterator it = v.begin(); it != v.end();it++)
		{
			std::pair< std::string, void* > *result = parse_property(*it);
			if(result != NULL)
			{
				
			}
			else
			{
				std::cout << "Error in parsing property " << *it << "\n";
				exit(EXIT_FAILURE);
			}
		}
	}
	
	random_generator = new base_generator_type(random_seed);

	Graph *g;
	//g = generate_graph(num_vertices,num_edges);
	//write_graphviz(std::cout, *g);
	//delete g;

	g = generate_graph_random_vertex_pairs(nb_vertices,nb_edges);
	write_graphviz(std::cout, *g);
	delete g;
	return 0;
}
