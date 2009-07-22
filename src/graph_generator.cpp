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
#include "graph_generator.hpp"

using namespace boost;

/******************************************************************************
 * Graph_generation_context
 *****************************************************************************/

Graph_generation_context::Graph_generation_context()
{
	rng = NULL;
}

Graph_generation_context::~Graph_generation_context()
{
	// we don't destroy it because the option parser still need it
	rng = NULL;
}

void Graph_generation_context::set_rng(ggen_rng* r)
{
	rng = r;
}

ggen_rng* Graph_generation_context::get_rng()
{
	return rng;
}


/******************************************************************************
 * Graph_generation
 *****************************************************************************/

		
/* 
* A method to convert an adjacency matrix to an object of type Graph.
*
* Run through the adjacency matrix
* and at each i,j decide if matrix[i][j] is an edge or not.
*/
Graph* Graph_generation::translate_matrix_to_a_graph( bool **matrix, vertices_size num_vertices)
{
	Graph *g = new Graph(num_vertices);
	std::map < vertices_size, Vertex > vmap;
	std::pair < Vertex_iter, Vertex_iter > vp;
	vertices_size i = 0;
	for(vp = boost::vertices(*g); vp.first != vp.second; vp.first++)
		vmap[i++] = *vp.first;

	for( i = 0;i < num_vertices; i++)
		for( vertices_size j = 0; j < num_vertices; j++) 
			if(matrix[i][j])
				add_edge(vmap[i],vmap[j],*g);
	
	return g;
}

/* Erdos-Renyi : G(n,p)
*/
Graph* Graph_generation::gg_erdos_gnp(Graph_generation_context &cntxt, vertices_size num_vertices, double p, bool do_dag)
{
	ggen_rng *rng = cntxt.get_rng(); 
	
	// generate the matrix
	bool **matrix = new bool *[num_vertices];
        
	for( vertices_size k = 0 ; k < num_vertices ; k++ )
		matrix[k] = new bool[num_vertices];
	vertices_size i, j;
	
	for(i = 0; i < num_vertices; i++)
	{
		for(j = 0; j < num_vertices ; j++)
		{
			// this test activate always if do_dag is false,
			// only if i < j if do_dag is true
			if(i < j || !do_dag)
			{
				// coin flipping to determine if we add an edge or not
				matrix[i][j] = rng->bernoulli(p);
			}
			else
				matrix[i][j] = false;
		}
	}

	// translate the matrix to a graph
	return translate_matrix_to_a_graph( (bool **) matrix, num_vertices);
}



/* Layer-by-Layer Method: 
* Using coin flipping to connect the layers
* Erdos method for connecting vertices
*/
Graph* 	Graph_generation::gg_layer_by_layer(Graph_generation_context &cntxt, vertices_size num_vertices, double p, bool do_dag,std::vector<int> layer_num_vertex)
{
 	ggen_rng* rng = cntxt.get_rng();
	//generate the matrix
	bool **matrix = new bool *[num_vertices];
        for( vertices_size k = 0 ; k < num_vertices ; k++ )
		matrix[k] = new bool[num_vertices];
        
	vertices_size i, j;
	
	for(i = 0; i < num_vertices; i++)
	{
		for(j = 0; j < num_vertices ; j++)
		{	
			/*this test activates if do_dag is false and the vertices i,j are not in the same layer
			 or if do_dag is true and the edge(i,j) povertices_sizes downwards.*/
			if((!do_dag&& layer_num_vertex[i]!=layer_num_vertex[j])||(do_dag&&layer_num_vertex[i]<layer_num_vertex[j]))
			{
				//coin flipping to determine if we add an edge or not
				matrix[i][j] = rng->bernoulli(p);
			}
			else
				matrix[i][j] = false;
		}
	}
	
	 //translate the matrix to a graph
	return translate_matrix_to_a_graph( (bool **)matrix, num_vertices);
}

/* Returns an array containing layer indices for all the vertices. This array is required for the random graph generation method "Layer-by-Layer".
* A random number between 1 and 'num_layers' is generated and is assigned to each vertex.
*/
std::vector <int> Graph_generation::layer_allocation(Graph_generation_context &cntxt,unsigned long int num_layers,vertices_size num_vertices)
{
              
	ggen_rng* rng = cntxt.get_rng(); 
	std::vector<int>layer_num_vertex;
	dbg(trace, "no.of layers is = %lu\n",num_layers);
               
   
	for(vertices_size i = 0;i < num_vertices; i++)                                   
	{	
		// Generating a random layer no.
		int layer_index = rng->uniform_int(num_layers); 
		 
		//storing the layer no. just generated into a vector
		layer_num_vertex.push_back(layer_index);                    
	}
                      
           
	dbg(trace,"vertex no..............layer_number\n");
	for(vertices_size i = 0;i < num_vertices; i++)
	{	 
		//printing the layer numbers for all the vertices
		dbg(trace,"%d\t\t%d\n",i,layer_num_vertex[i]);   
	}

	return layer_num_vertex;           
}

/* Task Graphs for free: 
*/
Graph* Graph_generation::gg_tgff(Graph_generation_context &cntxt,int lower_bound,int max_od,int max_id)
{

        ggen_rng *rng = cntxt.get_rng();
	Graph *g = new Graph();
	//Addition of Starting node i.e. 0th node
	Vertex temp1 = add_vertex(*g);  

	while(boost::num_vertices(*g) < lower_bound) {
        	if(rng->bernoulli(.5))     //Fan-out Step
		{            
			std::pair<Vertex_iter, Vertex_iter> vp;
			std::map <Vertex,int >available_od; 
			int max = -1;  
			/*Calculation of available out degree for each vertex and storing them in the map
			"available_od" and calculation of maximum available out_degree */
			for (vp = boost::vertices(*g); vp.first != vp.second; ++vp.first)
			{	int available_out_degree = max_od - out_degree(*vp.first,*g);
				available_od[*vp.first] = available_out_degree;
				if(available_out_degree >= max)
				max = available_out_degree;
			}
                                                                                                                 
			std::vector<Vertex>available_vertices;
			int i = 0;
			/*Collecting all the vertices with the  
			maximum available out_degree into the vector "available_vertices"*/
			for (vp = boost::vertices(*g); vp.first != vp.second; ++vp.first)
				if(available_od[*vp.first] == max)
					available_vertices.push_back(*vp.first);

			/*Choosing  randomly a vertex from the available_map*/                
			int random_vertex_index = rng->uniform_int(available_vertices.size());
			Vertex temp = available_vertices[random_vertex_index];
 
			/*Deciding randomly the no. of out_nodes between 1 & max*/
			int out_nodes = rng->uniform_int(max) + 1; 

			/*Introducing new nodes and edges between temp node and new introduced nodes*/
			std::vector< Vertex > new_vertex(out_nodes);
			std::vector< Edge > new_edge(out_nodes);
			bool inserted;
				for (int i = 0; i < out_nodes; i++)
				{		
					new_vertex[i] = add_vertex(*g);
					tie(new_edge[i], inserted) = add_edge(temp, new_vertex[i], *g);
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
			for (vp = boost::vertices(*g); vp.first != vp.second; ++vp.first)
			{	available_od[*vp.first] = max_od - out_degree(*vp.first,*g);
				if(available_od[*vp.first]>0) available_vertices.push_back(*vp.first);
			}
                                     
                                    
			Vertex temp = add_vertex(*g);
			int cardinality = available_vertices.size();
			if(cardinality > max_id) cardinality = max_id;
			int num_out_nodes = rng->uniform_int(cardinality)+1; 
                                   
                        
			/*Randomly picking up exactly "num_out_nodes" no. of vertices from the vector "available_vertices"*/
			boost::any *src = new boost::any[available_vertices.size()];
			boost::any *dest = new boost::any[num_out_nodes];
			i = 0;
			std::vector< Vertex >::iterator ptr;
			for (ptr = available_vertices.begin();ptr != available_vertices.end(); ++ptr)
				src[i++] = boost::any_cast<Vertex>(*ptr);
			rng->choose(dest,num_out_nodes,src,available_vertices.size(),sizeof(boost::any));
 
			std::vector < Edge > new_edge(num_out_nodes);
			bool inserted;
			for(int i=0;i<num_out_nodes;i++)
			{
				tie(new_edge[i],inserted)=add_edge(boost::any_cast<Vertex>(dest[i]),temp,*g);
				if(inserted==false)
				std::cout<<"Error in edge insertion"<<'\n';
			}
		}	 
	}            
	
	return g;
}
     
/* Erdos-Renyi : G(n,M)
*  
*/
Graph* Graph_generation::gg_erdos_gnm(Graph_generation_context &cntxt, vertices_size num_vertices, edges_size num_edges, bool do_dag) 
{
	ggen_rng* rng = cntxt.get_rng();
	bool **matrix = new bool *[num_vertices];
        
	for( vertices_size k = 0 ; k < num_vertices ; k++ )
		matrix[k] = new bool[num_vertices];
	vertices_size i, j;
        for(i = 0;i < num_vertices; i++)
		for(j = 0; j < num_vertices; j++)
			matrix[i][j] = 0;
        
        
        
	edges_size added_edges = 0;
	while(added_edges < num_edges) {
        	i = rng->uniform_int(num_vertices);
		j = rng->uniform_int(num_vertices);
		bool inserted;
 
                if((!do_dag && i != j && matrix[i][j] == 0) ||(do_dag && i < j && matrix[i][j] == 0)) 
                {
			matrix[i][j] = 1;
			added_edges++;        
                }
                
        }
       
                 
	// translate the matrix to a graph
	return translate_matrix_to_a_graph( (bool **)matrix, num_vertices );
}

/* Random Orders Method :
*/
Graph*  Graph_generation::gg_random_orders_method(Graph_generation_context &cntxt, int num_vertices, int num_pos)
{
	ggen_rng *rng = cntxt.get_rng();
	int i, j;
	int k;
	bool **matrix = new bool *[num_vertices];
        
	for( i = 0 ; i < num_vertices ; i++ )
		matrix[i] = new bool[num_vertices];
	int test_edge[num_vertices][num_vertices];
	

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
		rng-> shuffle (poset[k], num_vertices, sizeof (boost::any));

	//Making the array "index" to hold the indices of the vertices for every permutation
	int index[num_pos][num_vertices];

	for( k = 0; k < num_pos; k++)
		for( j = 0; j < num_vertices; j++)
			index[k][boost::any_cast<int>(poset[k][j])] = j;
	
	for( i = 0; i < num_vertices; i++)
		for( j = 0; j < num_vertices; j++)
			for( k = 0; k < num_pos; k++)
				if( index[k][i] < index[k][j])	
					test_edge[i][j]++;
	//if index of the vertex 'i' is less than index of vertex 'j' in every permutation, edge is introduced between (i,j)
	for( i = 0; i < num_vertices; i++)
		for( j = 0; j < num_vertices; j++)
			if( test_edge[i][j] == num_pos) matrix[i][j] = 1;

	// translate the matrix to a graph
	return translate_matrix_to_a_graph((bool **)matrix, num_vertices);	
}

////////////////////////////////////////////////////////////////////////////////
// MAIN
////////////////////////////////////////////////////////////////////////////////

namespace po = boost::program_options;
dynamic_properties properties(&create_property_map);

/** 
* Main program
*
*/
int main(int argc, char** argv)
{
	// Init the structures
	////////////////////////////

	Graph *g = NULL;
	Graph_generation_context *cntxt = new Graph_generation_context();
	
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
		std::cout << "erdos_gnp\t\tThe classical adjacency matrix method" << std::endl;
                std::cout << "layer_by_layer\t\tCoin flipping to connect the layers" << std::endl;
                std::cout << "tgff\t\t\tThe Task Graphs for Free method"<< std::endl;
		std::cout << "erdos_gnm\t\t" << std::endl;
		std::cout << "rom\t\t\tRandom Orders Method" << std::endl;
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
	ggen_rng *rng = random_rng_handle_options_atinit(vm_general);
	cntxt->set_rng(rng);


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
			vertices_size nb_vertices;
			// define the options specific to this method
			od_method.add_options()
				("dag",po::bool_switch(&do_dag)->default_value(false),"Generate a DAG instead of a classical graph")
				("nb-vertices,n",po::value<vertices_size>(&nb_vertices)->default_value(10),"Set the number of vertices in the generated graph")
				("probability,p",po::value<double>(&prob)->default_value(0.5),"The probability to get each edge");
                               
			// define method arguments as positional
			po::positional_options_description pod_method_args;
			pod_method_args.add("nb-vertices",1);
			pod_method_args.add("probability",1);
                        

			// do the parsing
			po::store(po::command_line_parser(to_parse).options(od_method).positional(pod_method_args).run(),vm_method);
			po::notify(vm_method);
			
			g = Graph_generation::gg_erdos_gnp(*cntxt,nb_vertices,prob,do_dag);
		}
                else if(method_name == "layer_by_layer")
		{
			bool do_dag;
			double prob;
			vertices_size nb_vertices;
                        int nb_layers;
			// define the options specific to this method
			od_method.add_options()
				("dag",po::bool_switch(&do_dag)->default_value(false),"Generate a DAG instead of a classical graph")
				("nb-vertices,n",po::value<vertices_size>(&nb_vertices)->default_value(10),"Set the no. of vertices in the generated graph")
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
                        
			std::vector<int>layer_num_vertex = Graph_generation::layer_allocation(*cntxt,nb_layers,nb_vertices);
			g = Graph_generation::gg_layer_by_layer(*cntxt,nb_vertices,prob,do_dag,layer_num_vertex);
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
                        
			g = Graph_generation::gg_tgff(*cntxt,lower_bound,max_od,max_id);
		}
		else if(method_name == "erdos_gnm")
		{
			bool do_dag;
			vertices_size nb_vertices;
			edges_size nb_edges;
			// define the options specific to this method
			od_method.add_options()
				("dag",po::bool_switch(&do_dag)->default_value(false),"Generate a DAG instead of a classical graph")
				("nb-vertices,n",po::value<vertices_size>(&nb_vertices)->default_value(10),"Set the no. of vertices in the generated graph")
				("nb-edges,n",po::value<edges_size>(&nb_edges)->default_value(10),"Set the number of edges in the generated graph");
			// define method arguments as positional
			po::positional_options_description pod_method_args;
			
			pod_method_args.add("nb-vertices",1);
			pod_method_args.add("nb-edges",1);
			
			// do the parsing
			po::store(po::command_line_parser(to_parse).options(od_method).positional(pod_method_args).run(),vm_method);
			po::notify(vm_method);
			
			g = Graph_generation::gg_erdos_gnm(*cntxt,nb_vertices,nb_edges,do_dag);
		}
		else if(method_name == "rom")
		{
			int nb_vertices;
			int nb_pos;
			// define the options specific to this method
			od_method.add_options()
				("nb-vertices,n",po::value<int>(&nb_vertices)->default_value(10),"Set the no. of vertices in the generated graph")
				("nb-pos,l",po::value<int>(&nb_pos)->default_value(5),"Set the number of posets in the graph");
			// define method arguments as positional
			po::positional_options_description pod_method_args;
			pod_method_args.add("nb-vertices",1);
			pod_method_args.add("nb-pos",1);
			
			// do the parsing
			po::store(po::command_line_parser(to_parse).options(od_method).positional(pod_method_args).run(),vm_method);
			po::notify(vm_method);
                        
			g = Graph_generation::gg_random_orders_method(*cntxt,nb_vertices,nb_pos);
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

	random_rng_handle_options_atexit(vm_general,rng);
	delete cntxt;
	delete g;
	return 0;
}
