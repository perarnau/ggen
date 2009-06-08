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
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

/* We use extensively the BOOST library for 
 * handling output, program options and random generators
 */
#include <boost/config.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/strong_components.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/program_options.hpp>

#include "../ggen.hpp"
#include "../dynamic_properties.hpp"

using namespace boost;
using namespace std;

dynamic_properties properties(&create_property_map);

////////////////////////////////////////////////////////////////////////////////
// MAIN
////////////////////////////////////////////////////////////////////////////////

namespace po = boost::program_options;



/* Main program
*/
int main(int argc, char** argv)
{
	Graph *g;
	unsigned int **matrix;
	unsigned int size;
	unsigned int i,j;

	// Handle command line arguments
	////////////////////////////////////
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce this help message")

		/* I/O options */
		("input,i", po::value<string>(), "Set the input file")
		("output,o",po::value<string>(), "Set the output file")
		
		/* Analysis options */
		("size,s", po::value<unsigned int>(&size)->default_value(10), "Size of the output matrix")
		("add,a", po::value<bool>()->zero_tokens(),"Add the result to the output matrix instead of erasing it")
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
		// open the file for reading
		int in = open(vm["input"].as<std::string>().c_str(),O_RDONLY);
	
		// redirect stdout to it
		dup2(in,STDIN_FILENO);
		close(in);
	}

	// we must initialize the matrix first
	matrix = new unsigned int*[size];
	for(i = 0; i < size ; i++)
		matrix[i] =  new unsigned int[size];

	// init matrix
	for(i = 0; i < size; i++)
		for(j = 0; j < size ; j++)
			matrix[i][j] = 0;


	if (vm.count("output")) 
	{
		if(vm.count("add"))
		{
			// read the output file first
			filebuf *fb = new filebuf();
			fb->open(vm["output"].as<std::string>().c_str(),ios::in);
			istream *infile = new istream(fb);
			for(i = 0; i < size; i++)
				for(j = 0; j < size ; j++)
					*infile >> matrix[i][j];

			delete infile;
		}
		
		// create a new file with 344 file permissions
		int out = open(vm["output"].as<std::string>().c_str(),O_WRONLY | O_CREAT | O_TRUNC,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );
	
		// redirect stdout to it
		dup2(out,STDOUT_FILENO);
		close(out);
	}


	// Graph generation
	////////////////////////////////

	g = new Graph();

	// Read graph
	////////////////////////////////	
	read_graphviz(std::cin, *g,properties);

	// Analyse the graph
	////////////////////////////////

	// Index map
	std::map < Vertex, unsigned int > imap;
	boost::associative_property_map< std::map< Vertex, unsigned int> > indexmap(imap);

	// Update map
	i = 0;
	std::pair<Vertex_iter, Vertex_iter> vp;
	for (vp = boost::vertices(*g); vp.first != vp.second; ++vp.first)
	{
		imap.insert(make_pair(*vp.first,i++));
	}

	// update matrix
	std::pair< Edge_iter, Edge_iter> ep;
	for (ep = boost::edges(*g); ep.first != ep.second; ++ep.first)
	{
		Vertex s,t;
		unsigned int k,l;
		s = source(*ep.first,*g);
		t = target(*ep.first,*g);
		k = imap[s];
		l = imap[t];
		matrix[k][l] += 1;
	}

	
	// output matrix
	for(i = 0; i < size; i++)
	{
		for(j = 0; j < size ; j++)
		{
			std::cout << matrix[i][j] << " ";
		}
		std::cout << std::endl;
	}
	delete g;
	return EXIT_SUCCESS;
}
