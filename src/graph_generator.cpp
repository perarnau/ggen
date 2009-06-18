/* Copyright Swann Perarnau 2009
*
*   contributor(s) : 
*	Pradeep Beniwal (pdpbeniwal AT gmail.com)
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
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

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
// Global definitions
////////////////////////////////////////////////////////////////////////////////

// a random number generator for all our random stuff, initialized in main
ggen_rng* global_rng;

////////////////////////////////////////////////////////////////////////////////
// Generation Methods
////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////
// Erdos-Renyi : G(n,p)
// One of the simplest way of generating a graph
// Supports :
// 	-- dag option
// 	-- params : number of vertices, probability of an edge
///////////////////////////////////////

/* Run through the adjacency matrix
 * and at each i,j decide if matrix[i][j] is an edge with a given probability
 */
void gg_erdos_gnp(Graph& g, int num_vertices, double p, bool do_dag)
{
	// generate the matrix
	bool matrix[num_vertices][num_vertices];
	int i,j;
	
	for(i=0; i < num_vertices; i++)
	{
		for(j = 0; j < num_vertices ; j++)
		{
			// this test activate always if do_dag is false,
			// only if i < j if do_dag is true
			if(i < j || !do_dag)
			{
				// coin flipping to determine if we add an edge or not
				matrix[i][j] = global_rng->bernoulli(p);
			}
			else
				matrix[i][j] = false;
		}
	}

	// translate the matrix to a graph
	g = Graph(num_vertices);
	std::map < int, Vertex > vmap;
	std::pair < Vertex_iter, Vertex_iter > vp;
	i = 0;
	for(vp = boost::vertices(g); vp.first != vp.second; vp.first++)
		vmap[i++] = *vp.first;

	for(i=0;i < num_vertices; i++)
		for(j=0; j < num_vertices; j++)
			if(matrix[i][j])
				add_edge(vmap[i],vmap[j],g);

}

///////////////////////////////////////
// Layer-by-Layer Method
// Using coin flipping to connect the layers
// Supports :
// 	-- dag option
// 	-- params : number of vertices, probability of an edge,number of layers
///////////////////////////////////////

/* Run through the adjacency matrix
 * and at each i,j decide if matrix[i][j] is an edge with a given probability and no edge is formed beteen the two vertices lying in the same layer
 */
void gg_layer_by_layer(Graph& g, int num_vertices, double p, bool do_dag,std::vector<int> layer_num_vertex)
{
	// generate the matrix
	bool matrix[num_vertices][num_vertices];
	int i,j;
	
	for(i=0; i < num_vertices; i++)
	{
		for(j = 0; j < num_vertices ; j++)
		{
			// this test activates if do_dag is false and the vertices i,j are not in the same layer
			// or if do_dag is true and the edge(i,j) points downwards.
			if((!do_dag&& layer_num_vertex[i]!=layer_num_vertex[j])||(do_dag&&layer_num_vertex[i]<layer_num_vertex[j]))
			{
				// coin flipping to determine if we add an edge or not
				matrix[i][j] = global_rng->bernoulli(p);
			}
			else
				matrix[i][j] = false;
		}
	}

	// translate the matrix to a graph
	g = Graph(num_vertices);
	std::map < int, Vertex > vmap;
	std::pair < Vertex_iter, Vertex_iter > vp;
	i = 0;
	for(vp = boost::vertices(g); vp.first != vp.second; vp.first++)
		vmap[i++] = *vp.first;


	for(i=0;i < num_vertices; i++)
		for(j=0; j < num_vertices; j++)
			if(matrix[i][j])
				add_edge(vmap[i],vmap[j],g);

}

//To generate a vector containing layer no. of each vertex of the graph
std::vector<int> layer_allocation(unsigned long int num_layers,int num_vertices)
{
              
	      
	std::vector<int>layer_num_vertex;
	dbg(trace,"no.of layers is= %lu\n",num_layers);
               
   
	for(int i=0;i<num_vertices;i++)                                   
	{
		int layer_index = global_rng->uniform_int(num_layers); //Generating a random layer no.
		layer_num_vertex.push_back(layer_index);                     //storing the layer no. just generated into a vector
	}
                      
           
	dbg(trace,"vertex no..............layer_number\n");
	for(int i=0;i<num_vertices;i++)
	{
		dbg(trace,"%d\t\t%d\n",i,layer_num_vertex[i]);   //printing the layer numbers for all the vertices
	}

	return layer_num_vertex;           
}

// Task Graphs for free: tgff
// 	-- params : a lower bound on the number of vertices,maximum out_degree limit and maximum in_degree limit
///////////////////////////////////////

/* The Task Graphs for Free method carries out graph generation by iteratively incorporating the two steps i.e fan-out and fan-in
both these steps occur with the equal probability=0.5 */

void gg_tgff(Graph& g,int lower_bound,int max_od,int max_id)
{

        //Addition of Starting node i.e. 0th node
	Vertex temp1 = add_vertex(g);  

	while(boost::num_vertices(g) < lower_bound) {
        	if(global_rng->bernoulli(.5))     //Fan-out Step
		{            
			std::pair<Vertex_iter, Vertex_iter> vp;
			std::map <Vertex,int >available_od; 
			int max = -1;  
			/*Calculation of available out degree for each vertex and storing them in the map
			"available_od" and calculation of maximum available out_degree */
			for (vp = boost::vertices(g); vp.first != vp.second; ++vp.first)
			{	int available_out_degree = max_od - out_degree(*vp.first,g);
				available_od[*vp.first] = available_out_degree;
				if(available_out_degree >= max)
				max = available_out_degree;
			}
                                                                                                                 
			std::vector<Vertex>available_vertices;
			int i=0;
			/*Collecting all the vertices with the  
			maximum available out_degree into the vector "available_vertices"*/
			for (vp = boost::vertices(g); vp.first != vp.second; ++vp.first)
				if(available_od[*vp.first] == max)
					available_vertices.push_back(*vp.first);

			/*Choosing  randomly a vertex from the available_map*/                
			int random_vertex_index = global_rng->uniform_int(available_vertices.size());
			Vertex temp = available_vertices[random_vertex_index];
 
			/*Deciding randomly the no. of out_nodes between 1 & max*/
			int out_nodes = global_rng->uniform_int(max) + 1; 

			/*Introducing new nodes and edges between temp node and new introduced nodes*/
			std::vector< Vertex > new_vertex(out_nodes);
			std::vector< Edge > new_edge(out_nodes);
			bool inserted;
				for (int i = 0; i < out_nodes; i++)
				{		
					new_vertex[i] = add_vertex(g);
					tie(new_edge[i], inserted) = add_edge(temp, new_vertex[i], g);
					if(inserted==false)
					std::cout<<"Error in edge insertion"<<'\n';
                                                
				}
          
                                                       
		}   
                                                    
		else	//Fan-In Step
		{                       
			std::pair<Vertex_iter, Vertex_iter> vp;
			std::map <Vertex, int >available_od;
			int i = 0;
			std::vector< Vertex > available_vertices;
			/*Collecting all the vertices with available out_degree greater than 
			 zero into the vector "available_vertices" */                   
			for (vp = boost::vertices(g); vp.first != vp.second; ++vp.first)
			{	available_od[*vp.first] = max_od - out_degree(*vp.first,g);
				if(available_od[*vp.first]>0) available_vertices.push_back(*vp.first);
			}
                                     
                                    
			Vertex temp = add_vertex(g);
			int cardinality = available_vertices.size();
			if(cardinality > max_id) cardinality = max_id;
			int num_out_nodes = global_rng->uniform_int(cardinality)+1; 
                                   
                        
			/*Randomly picking up exactly "num_out_nodes" no. of vertices from the vector "available_vertices"*/
			boost::any *src = new boost::any[available_vertices.size()];
			boost::any *dest = new boost::any[num_out_nodes];
			i = 0;
			std::vector< Vertex >::iterator ptr;
			for (ptr = available_vertices.begin();ptr != available_vertices.end(); ++ptr)
				src[i++] = boost::any_cast<Vertex>(*ptr);
			global_rng->choose(dest,num_out_nodes,src,available_vertices.size(),sizeof(boost::any));
 
			std::vector < Edge > new_edge(num_out_nodes);
			bool inserted;
			for(int i=0;i<num_out_nodes;i++)
			{
				tie(new_edge[i],inserted)=add_edge(boost::any_cast<Vertex>(dest[i]),temp,g);
				if(inserted==false)
				std::cout<<"Error in edge insertion"<<'\n';
			}
		}	 
	}            

}
     
///////////////////////////////////////
// Erdos-Renyi : G(n,M)
// One of the simplest way of generating a graph
// Supports :
// 	-- dag option
// 	-- params : number of vertices,number of edges
///////////////////////////////////////

/* Run through the adjacency matrix
 * and at each i,j decide if matrix[i][j] is an edge with a given probability
 */
void gg_erdos_gnm(Graph& g, int num_vertices, int num_edges, bool do_dag) {
        bool matrix[num_vertices][num_vertices];
        int i,j;
        for(i = 0;i < num_vertices; i++)
		for(j = 0; j < num_vertices; j++)
			matrix[i][j] = 0;
        
        
        
	int added_edges = 0;
	while(added_edges < num_edges) {
        	i = global_rng->uniform_int(num_vertices);
		j = global_rng->uniform_int(num_vertices);
		bool inserted;
 
                if((!do_dag && i != j && matrix[i][j] == 0) ||(do_dag && i < j && matrix[i][j] == 0)) 
                {
			matrix[i][j] = 1;
			added_edges++;        
                }
                
        }
       
                 
       // translate the matrix to a graph
	g = Graph(num_vertices);
	std::map < int, Vertex > vmap;
	std::pair < Vertex_iter, Vertex_iter > vp;
	i = 0;
	for(vp = boost::vertices(g); vp.first != vp.second; vp.first++)
		vmap[i++] = *vp.first;


	for(i = 0; i < num_vertices; i++)
		for(j = 0; j < num_vertices; j++)
			if(matrix[i][j])
				add_edge(vmap[i], vmap[j], g);
                
}


///////////////////////////////////////
// Random Orders Method : f(num_vertices,num_pos)
// One of the simplest way of generating a graph
// Supports :
// 	-- params : number of vertices,number of partially ordered sets
///////////////////////////////////////

/* It makes permutations of the vertices and 
 * if index of the vertex 'i' is less than index of vertex 'j' in every permutation, 
   an edge is introduced between the vertex 'i' and the vertex 'j'.
 */
void gg_random_orders_method(Graph& g, int num_vertices, int num_pos, int d)
{
	bool matrix[num_vertices][num_vertices];
	int test_edge[num_vertices][num_vertices];
	int i, j, k;

	//Nullifying the arrays "matrix" and "test_edge"
	for( i = 0; i < num_vertices; i++)
		for( j = 0; j < num_vertices; j++)
		{	test_edge[i][j] = 0;
			matrix[i][j] = 0;
		}
	//Making an array named "poset" to hold the permutations of the vertices
	boost::any **poset = new boost::any*[num_pos];
        
	for( k = 0 ; k < num_pos ; k++ )
		poset[k] = new boost::any[num_vertices];

 
	for( k = 0; k < num_pos; k++)
		for( j = 0; j < num_vertices; j++)
			poset[k][j] = j;

	//Shuffling all the rows of the array "poset"
	for( k = 0; k < num_pos; k++)
		global_rng-> shuffle (poset[k], num_vertices, sizeof (boost::any));

	//Making the array "index" to hold the indices of the vertices for every permutation
	int index[num_pos][num_vertices];

	for( i = 0; i < num_pos; i++)
		for( j = 0; j < num_vertices; j++)
			index[i][boost::any_cast<int>(poset[i][j])] = j;
	
	for( i = 0; i < num_vertices; i++)
		for( j = 0; j < num_vertices; j++)
			for( k = 0; k < num_pos; k++)
				if( index[k][i]-index[k][j] <= d && index[k][i]-index[k][j] > 0)	
					test_edge[i][j]++;
				else if(d == 0 && i != j) 
					matrix[i][j] = 1;
	//if index of the vertex 'i' is less than index of vertex 'j' in every permutation, edge is introduced between (i,j)
	for( i = 0; i < num_vertices; i++)
		for( j = 0; j < num_vertices; j++)
			if( test_edge[i][j] == num_pos) matrix[i][j] = 1;

	 // translate the matrix to a graph
	g = Graph(num_vertices);
	std::map < int, Vertex > vmap;
	std::pair < Vertex_iter, Vertex_iter > vp;
	i = 0;
	for(vp = boost::vertices(g); vp.first != vp.second; vp.first++)
		vmap[i++] = *vp.first;


	for(i = 0; i < num_vertices; i++)
		for(j = 0; j < num_vertices; j++)
			if(matrix[i][j])
				add_edge(vmap[i], vmap[j], g);
	
	
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
	// Init the structures
	////////////////////////////

	g = new Graph();
	
	// Handle command line arguments
	////////////////////////////////////
	po::options_description od_general("General Options");
	od_general.add_options()
		("help", "produce help message")

		/* I/O options */
		("output,o", po::value<std::string>(), "Set the output file")
	;
	
	ADD_DBG_OPTIONS(od_general);

	po::options_description od_methods("Methods Options");
	od_methods.add_options()
		("method", po::value<std::string>(),"The generation method to use")
		("method-args",po::value<std::vector<std::string> >(),"The generation method's arguments")
	;


	// Positional Options
	///////////////////////////////

	po::positional_options_description pod_methods;
	pod_methods.add("method", 1);
	pod_methods.add("method-args",-1);

	po::options_description od_all;
	po::options_description od_ro = random_rng_options();

	od_all.add(od_general).add(od_methods).add(od_ro);

	po::variables_map vm_general;
	po::parsed_options prso_general = po::command_line_parser(argc,argv).options(od_all).positional(pod_methods).allow_unregistered().run();
	po::store(prso_general,vm_general);
	po::notify(vm_general);
	
	if (vm_general.count("help")) {
		std::cout << "Usage: " << argv[0] << "[options] method_name method_arguments" << std::endl << std::endl;

		std::cout << od_general << std::endl;
		std::cout << od_ro << std::endl;
		
		std::cout << "Methods Available:" << std::endl;
		std::cout << "erdos_gnp\t\tThe classical adjacency matrix method" << std::endl << std::endl;
                std::cout << "layer_by_layer\t\tCoin flipping to connect the layers" << std::endl;
                std::cout << "tgff\t\t\tThe Task Graphs for Free method"<< std::endl << std::endl;
		std::cout << "erdos_gnm\t\t" << std::endl << std::endl;
		std::cout << "Random Orders Method\t\t" << std::endl;
	return 1;
	}
	
	if (vm_general.count("output")) 
	{
		// create a new file with 344 file permissions
		int out = open(vm_general["output"].as<std::string>().c_str(),O_WRONLY | O_CREAT | O_TRUNC,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );
	
		// redirect stdout to it
		dup2(out,STDOUT_FILENO);
		close(out);
	}

	// Random number generator, options handling
	global_rng = random_rng_handle_options_atinit(vm_general);



	// Graph methods handling
	////////////////////////////////
	
	// First we recover all the possible options we didn't parse
	// this is the -- options that we didn't recognize.
	std::vector< std::string > unparsed_args = po::collect_unrecognized(prso_general.options,po::exclude_positional);
	// this is the positional arguments that were given to the method
	std::vector< std::string > parsed_args;
	if(vm_general.count("method-args"))
		parsed_args = vm_general["method-args"].as < std::vector < std::string > >();
	
	//we merge the whole thing
	std::vector< std::string > to_parse;
	to_parse.insert(to_parse.end(),unparsed_args.begin(),unparsed_args.end());
	to_parse.insert(to_parse.end(),parsed_args.begin(),parsed_args.end());

	

	if(vm_general.count("method"))
	{
		std::string method_name = vm_general["method"].as<std::string>();
		po::options_description od_method("Method options");
		po::variables_map vm_method;
		if(method_name == "erdos_gnp")
		{
			bool do_dag;
			double prob;
			int nb_vertices;
			// define the options specific to this method
			od_method.add_options()
				("dag",po::bool_switch(&do_dag)->default_value(false),"Generate a DAG instead of a classical graph")
				("nb-vertices,n",po::value<int>(&nb_vertices)->default_value(10),"Set the number of vertices in the generated graph")
				("probability,p",po::value<double>(&prob)->default_value(0.5),"The probability to get each edge");
                               
			// define method arguments as positional
			po::positional_options_description pod_method_args;
			pod_method_args.add("nb-vertices",1);
			pod_method_args.add("probability",1);
                        

			// do the parsing
			po::store(po::command_line_parser(to_parse).options(od_method).positional(pod_method_args).run(),vm_method);
			po::notify(vm_method);
			
			gg_erdos_gnp(*g,nb_vertices,prob,do_dag);
		}
                else if(method_name == "layer_by_layer")
		{
			bool do_dag;
			double prob;
			int nb_vertices;
                        int nb_layers;
			// define the options specific to this method
			od_method.add_options()
				("dag",po::bool_switch(&do_dag)->default_value(false),"Generate a DAG instead of a classical graph")
				("nb-vertices,n",po::value<int>(&nb_vertices)->default_value(10),"Set the number of vertices in the generated graph")
                                ("probability,p",po::value<double>(&prob)->default_value(0.5),"The probability to get each edge")
                                ("nb-layers,l",po::value<int>(&nb_layers)->default_value(5),"Set the number of layers in the graph");		
			// define method arguments as positional
			po::positional_options_description pod_method_args;
			pod_method_args.add("nb-vertices",1);
			pod_method_args.add("probability",1);
                        pod_method_args.add("nb-layers",1);
			
			// do the parsing
			po::store(po::command_line_parser(to_parse).options(od_method).positional(pod_method_args).run(),vm_method);
			po::notify(vm_method);
                        
			std::vector<int>layer_num_vertex=layer_allocation(nb_layers,nb_vertices);
			gg_layer_by_layer(*g,nb_vertices,prob,do_dag,layer_num_vertex);
		}
		else if(method_name == "tgff")
		{
			
			
			int lower_bound;
			int max_od;
			int max_id;
                        
			// define the options specific to this method
			od_method.add_options()
				("lower-bound",po::value<int>(&lower_bound)->default_value(10),"Set the value of the lower bound on the vertices")
				("max-od",po::value<int>(&max_od)->default_value(3),"Set the maximum out_degree limit for all the vertices")
				("max-id",po::value<int>(&max_id)->default_value(3),"Set the maximum in_degree limit for all the vertices");		
			// define method arguments as positional
			po::positional_options_description pod_method_args;
			pod_method_args.add("lower-bound",1);
			pod_method_args.add("max-od",1);
			pod_method_args.add("max-id",1);
			
			// do the parsing
			po::store(po::command_line_parser(to_parse).options(od_method).positional(pod_method_args).run(),vm_method);
			po::notify(vm_method);
                        
			gg_tgff(*g,lower_bound,max_od,max_id);
		}
		else if(method_name == "erdos_gnm")
		{
			bool do_dag;
			int nb_vertices;
			int nb_edges;
			// define the options specific to this method
			od_method.add_options()
				("dag",po::bool_switch(&do_dag)->default_value(false),"Generate a DAG instead of a classical graph")
				("nb-vertices,n",po::value<int>(&nb_vertices)->default_value(10),"Set the number of vertices in the generated graph")
				("nb-edges,n",po::value<int>(&nb_edges)->default_value(10),"Set the number of edges in the generated graph");
			// define method arguments as positional
			po::positional_options_description pod_method_args;
			
			pod_method_args.add("nb-vertices",1);
			pod_method_args.add("nb-edges",1);
			
			// do the parsing
			po::store(po::command_line_parser(to_parse).options(od_method).positional(pod_method_args).run(),vm_method);
			po::notify(vm_method);
			
			gg_erdos_gnm(*g,nb_vertices,nb_edges,do_dag);
		}
		 else if(method_name == "rom")
		{
			int nb_vertices;
			int nb_pos;
			int distance;
			// define the options specific to this method
			od_method.add_options()
				("nb-vertices,n",po::value<int>(&nb_vertices)->default_value(10),"Set the number of vertices in the generated graph")
				("nb-pos,l",po::value<int>(&nb_pos)->default_value(5),"Set the number of layers in the graph")
				("distance,l",po::value<int>(&distance)->default_value(2),"Set the distance between the verticesin the graph");		
			// define method arguments as positional
			po::positional_options_description pod_method_args;
			pod_method_args.add("nb-vertices",1);
			pod_method_args.add("nb-pos",1);
			pod_method_args.add("distance",1);
			
			// do the parsing
			po::store(po::command_line_parser(to_parse).options(od_method).positional(pod_method_args).run(),vm_method);
			po::notify(vm_method);
                        
			gg_random_orders_method(*g,nb_vertices,nb_pos,distance);
		}

		else
		{
			std::cerr << "Error : you must provide a VALID method name!" << std::endl;
			exit(1);
		}
	}
	else
	{
		std::cerr << "Error : you must provide a method name !" << std::endl;
		exit(1);
	}

	
	// since we created the graph from scratch we need to add a property for the vertices
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

	random_rng_handle_options_atexit(vm_general,global_rng);
	
	delete g;
	return 0;
}
