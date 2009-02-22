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
/*
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

*/


void add_vertex_property(ggen_rnd* rnd,string name)
{
	// iterators
	typedef std::map< graph_traits<Graph>::vertex_descriptor, double > user_map;
	typedef graph_traits<Graph>::vertex_iterator vertex_iter;
	typedef boost::associative_property_map< user_map > vertex_map;

	user_map* map = new user_map();
	vertex_map * bmap = new vertex_map(*map);
	properties.property(name,*bmap);
		
	// iterate and add random property
	std::pair<vertex_iter, vertex_iter> vp;
	for (vp = boost::vertices(*g); vp.first != vp.second; ++vp.first)
		    put(name,properties,*vp.first,rnd->get());
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

	// Handle command line arguments
	////////////////////////////////////
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		
		/* I/O options */
		("input,i", po::value<string>(&infilename), "Set the output file")
		("output,o", po::value<string>(&outfilename), "Set the output file")

		/* Property options */
		("name,n",po::value<string>(&name),"Set the property name")
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
	add_vertex_property(rs.rnd,name);
	
	// Write graph
	////////////////////////////////////	
	write_graphviz(*outfile, *g,properties);
	
	random_options_end(vm,rs);

	delete g;
	return 0;
}
