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
#include "random.hpp"

using namespace boost;
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// MAIN
////////////////////////////////////////////////////////////////////////////////

namespace po = boost::program_options;
dynamic_properties properties;
Graph *g;

void add_vertex_property(ggen_rnd* rnd,string name)
{
	// iterators
	typedef std::map< graph_traits<Graph>::vertex_descriptor, std::string > user_map;
	typedef graph_traits<Graph>::vertex_iterator vertex_iter;
	typedef boost::associative_property_map< user_map > vertex_map;

	user_map* map = new user_map();
	vertex_map * bmap = new vertex_map(*map);
	properties.property(name,*bmap);
		
	// iterate and add random property
	std::pair<vertex_iter, vertex_iter> vp;
	for (vp = boost::vertices(*g); vp.first != vp.second; ++vp.first)
		    put(name,properties,*vp.first,boost::lexical_cast<std::string>(rnd->get()));
}


// UGLY & BAD HACK : we need to create a map with edge_descriptor as Key but this
// type doesn't implement the required '<' operator.
// see http://www.nabble.com/BGL:-std::map<Edge,-int>-needs-the-<-operator-td4019596.html  for more details

struct cmp_edge :
	public std::binary_function<graph_traits<Graph>::edge_descriptor,graph_traits<Graph>::edge_descriptor, bool>
{
		bool operator()(const graph_traits<Graph>::edge_descriptor &e1, const graph_traits<Graph>::edge_descriptor &e2) const
		{
			return e1.get_property() < e2.get_property();
		}
}; 


void add_edge_property(ggen_rnd* rnd,string name)
{
	// iterators
	typedef std::map < graph_traits<Graph>::edge_descriptor, std::string, cmp_edge > user_map;
	typedef graph_traits<Graph>::edge_iterator edge_iter;
	typedef boost::associative_property_map< user_map > edge_map;

	user_map* map = new user_map();
	edge_map * bmap = new edge_map(*map);
	properties.property(name,*bmap);

	// iterate and add random property
	std::pair<edge_iter, edge_iter> ep;
	for (ep = boost::edges(*g); ep.first != ep.second; ++ep.first)
		    put(name,properties,*ep.first,boost::lexical_cast<std::string>(rnd->get()));
	
}


/* Main program
 */
int main(int argc, char** argv)
{
	int nb_vertices,nb_edges;
	string infilename, outfilename;
	string name;
	istream *infile = NULL;
	ostream *outfile = NULL;
	random_options_state rs;
	bool edge_property;

	// Handle command line arguments
	////////////////////////////////////
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		
		/* I/O options */
		("input,i", po::value<string>(), "Set the output file")
		("output,o", po::value<string>(), "Set the output file")

		/* Property options */
		("name,n",po::value<string>(),"Set the property name")
		("edge,e",po::bool_switch(),"Add an edge property instead of a vertex one")
	;

	po::options_description ro = random_add_options();
	po::options_description all;
	all.add(desc).add(ro);
	
	
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
		filebuf *fb = new filebuf();
		fb->open(vm["input"].as<std::string>().c_str(),ios::in);
		infile = new istream(fb);
	}
	else
		infile = &cin;
	
	if (vm.count("output")) 
	{
		filebuf *fb = new filebuf();
		fb->open(vm["output"].as<std::string>().c_str(),ios::out);
		outfile = new ostream(fb);
	}
	else
		outfile = &cout;

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


	random_options_start(vm,rs);

	// Graph generation
	////////////////////////////////
	
	g = new Graph();

	// Handle default property needed for dot parsing
	create_default_vertex_property(properties,*g);
	
	// Read graph
	////////////////////////////////////	
	read_graphviz(*infile, *g,properties);

	
	// Add property
	////////////////////////////////////
	if(edge_property)
		add_edge_property(rs.rnd,name);
	else
		add_vertex_property(rs.rnd,name);
	
	// Write graph
	////////////////////////////////////	
	write_graphviz(*outfile, *g,properties);
	
	random_options_end(vm,rs);

	delete g;
	return 0;
}
