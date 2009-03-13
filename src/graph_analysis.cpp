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
#include <climits>

/* We use extensively the BOOST library for 
 * handling output, program options and random generators
 */
#include <boost/config.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/program_options.hpp>

// lets try using gsl_histograms
#include <gsl/gsl_histogram.h>

#include "ggen.hpp"
#include "dynamic_properties.hpp"

using namespace boost;
using namespace std;

dynamic_properties properties(&create_property_map);
////////////////////////////////////////////////////////////////////////////////
// Analysis Function
////////////////////////////////////////////////////////////////////////////////

void minimum_spanning_tree(Graph g)
{
	Vertex source = *vertices(g).first;
	// We need a vector to store the spanning tree 
	std::vector < Vertex > p(num_vertices(g));

	// Weight map
	std::map < Edge, int, cmp_edge> wmap;
	boost::associative_property_map< std::map< Edge, int, cmp_edge > > weightmap(wmap);
	std::pair<Edge_iter, Edge_iter> ep;
	for (ep = boost::edges(g); ep.first != ep.second; ++ep.first)
	{
		wmap.insert(make_pair(*ep.first,1));
	}

	// Index map
	int i = 0;
	std::map < Vertex, int > imap;
	boost::associative_property_map< std::map< Vertex, int> > indexmap(imap);
	std::map< Vertex, int > dmap;
	boost::associative_property_map< std::map < Vertex, int> > distancemap(dmap);

	// Update maps
	std::pair<Vertex_iter, Vertex_iter> vp;
	for (vp = boost::vertices(g); vp.first != vp.second; ++vp.first)
	{
		imap.insert(make_pair(*vp.first,i++));
		dmap.insert(make_pair(*vp.first,0));
	}
	
	// Find a source a this graph
	for (vp = boost::vertices(g); vp.first != vp.second; ++vp.first)
	{
		std::pair< In_edge_iter, In_edge_iter> ip = in_edges(*vp.first,g);
		if(ip.first == ip.second)
		{
			source = *vp.first;
			break;
		}
	}

	//compute MST	
	prim_minimum_spanning_tree(g,source,&p[0],distancemap,weightmap,indexmap,default_dijkstra_visitor());

	// output the MST
	std::cout << "Minimum Spanning Tree:" << std::endl;
	std::cout << "source: " << source << std::endl;
	for (std::size_t i = 0; i != p.size(); ++i)
		if (p[i] != i)
			std::cout << "parent[" << i << "] = " << p[i] << std::endl;
		else
			std::cout << "parent[" << i << "] = no parent" << std::endl;

}

// compute several standard statistics like mean degree, ...
void degree_stats(Graph g)
{
	// allocate an histogram to accumulate data
	gsl_histogram *h = gsl_histogram_alloc(num_vertices(g));
	gsl_histogram_set_ranges_uniform(h,0,num_edges(g));

	// add each degree of each vertex
	std::pair< Vertex_iter, Vertex_iter> vp;
	for (vp = boost::vertices(g); vp.first != vp.second; ++vp.first)
	{
		gsl_histogram_increment(h,out_degree(*vp.first,g));
	}
	
	// print basic stats
	std::cout << "Degree Distribution:" 			<< std::endl;
	std::cout << "Min: "	<< gsl_histogram_min_val(h) 	<< std::endl;
	std::cout << "Max: "	<< gsl_histogram_max_val(h) 	<< std::endl;
	std::cout << "Mean: "	<< gsl_histogram_mean(h) 	<< std::endl;
	std::cout << "Std Dev: "<< gsl_histogram_sigma(h) 	<< std::endl;


	// print histogram
	std::cout << "Histogram:" << std::endl;
	gsl_histogram_fprintf (stdout, h, "%g", "%g");
	
	gsl_histogram_free(h);
}

// critical path
void longest_path(Graph g)
{
	// Index map
	int i = 0;
	std::map < Vertex, int > imap;
	std::map< Vertex, int> lpath;
	boost::associative_property_map< std::map< Vertex, int> > indexmap(imap);

	// Update map
	std::pair<Vertex_iter, Vertex_iter> vp;
	for (vp = boost::vertices(g); vp.first != vp.second; ++vp.first)
	{
		imap.insert(make_pair(*vp.first,i++));
		lpath.insert(make_pair(*vp.first,0));
	}
	
	// result
	std::list< Vertex> list;
	std::insert_iterator< std::list<Vertex> > il(list,list.begin());

	// sort topologically 
	boost::topological_sort(g,il);
	
	std::map< Vertex, Vertex> pmap;
	Vertex maxv;
	std::list< Vertex >::reverse_iterator it;
	for(it = list.rbegin(),maxv=*it; it != list.rend(); ++it)
	{
		// for each successor of this node, test if we are the best successor	
		std::pair< Out_edge_iter, Out_edge_iter> ep;
		for(ep= boost::out_edges(*it,g); ep.first != ep.second; ++ep.first)
		{
			Vertex w = target(*ep.first,g);
			if(lpath[w] < lpath[*it] + 1)
			{
				lpath[w] = lpath[*it] + 1;
				pmap[w] = *it;
			}
			if(lpath[w] > lpath[maxv])
				maxv = w;
		}
	}
	
	// Longest path
	std::cout << "Longest Path:" << std::endl;
	std::cout << "Size: " << lpath[maxv] << std::endl;
	Vertex s;
	while(pmap.find(maxv) != pmap.end())
	{
		s = pmap[maxv];
		std::cout << s << " -> " << maxv << std::endl;
		maxv = s;
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
	istream *infile = NULL;

	// Handle command line arguments
	////////////////////////////////////
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce this help message")

		/* I/O options */
		("input,i", po::value<string>(), "Set the input file")
		
		/* Analysis options */
		("nb-vertices,n", po::value<bool>()->zero_tokens(), "Output the number of vertices in the graph")
		("nb-edges,m", po::value<bool>()->zero_tokens(), "Output the number of edges in the graph")
		("degree-stats,d", po::value<bool>()->zero_tokens(),"Compute various statistics of the vertices degrees")
		("mst", po::value<bool>()->zero_tokens(),"Compute the Minimum Spanning Tree of the graph")
		("lp", po::value<bool>()->zero_tokens(),"Compute the Longest Path of the graph")
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
		filebuf *fb = new filebuf();
		fb->open(vm["input"].as<std::string>().c_str(),ios::in);
		infile = new istream(fb);
	}
	else
		infile = &cin;

	// Graph generation
	////////////////////////////////

	g = new Graph();

	// Read graph
	////////////////////////////////	
	read_graphviz(*infile, *g,properties);


	// Analyse the graph
	////////////////////////////////
	if(vm.count("nb-vertices"))
	{
		std::cout << "Nb Vertices: " << num_vertices(*g) << std::endl;
	}

	if(vm.count("nb-edges"))
	{
		std::cout << "Nb Edges: " << num_edges(*g) << std::endl;
	}

	if(vm.count("degree-stats"))
	{
		degree_stats(*g);
	}

	if(vm.count("mst"))
	{
		minimum_spanning_tree(*g);
	}
	if(vm.count("lp"))
	{
		longest_path(*g);
	}

	return EXIT_SUCCESS;
}
