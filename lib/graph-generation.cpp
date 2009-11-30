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

#include "types.hpp"
#include "random.hpp"
#include "dynamic_properties.hpp"
#include "graph-generation.hpp"

using namespace boost;
namespace ggen {
/******************************************************************************
 * generation_context
 *****************************************************************************/

generation_context::generation_context()
{
	rng = NULL;
}

generation_context::~generation_context()
{
	// we don't destroy it because the option parser still need it
	rng = NULL;
}

void generation_context::set_rng(ggen_rng* r)
{
	rng = r;
}

ggen_rng* generation_context::get_rng()
{
	return rng;
}


/******************************************************************************
 * generation
 *****************************************************************************/

		
/* 
* A method to convert an adjacency matrix to an object of type Graph.
*
* Run through the adjacency matrix
* and at each i,j decide if matrix[i][j] is an edge or not.
*/
Graph* generation::translate_matrix_to_a_graph( bool **matrix, vertices_size num_vertices)
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
Graph* generation::erdos_gnp(generation_context &cntxt, vertices_size num_vertices, double p, bool do_dag)
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
Graph* 	generation::layer_by_layer(generation_context &cntxt, vertices_size num_vertices, double p, bool do_dag,std::vector<int> layer_num_vertex)
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
std::vector <int> generation::layer_allocation(generation_context &cntxt,unsigned long int num_layers,vertices_size num_vertices)
{
              
	ggen_rng* rng = cntxt.get_rng(); 
	std::vector<int>layer_num_vertex;
   
	for(vertices_size i = 0;i < num_vertices; i++)                                   
	{	
		// Generating a random layer no.
		int layer_index = rng->uniform_int(num_layers); 
		 
		//storing the layer no. just generated into a vector
		layer_num_vertex.push_back(layer_index);                    
	}

	return layer_num_vertex;           
}

/* Task Graphs for free: 
*/
Graph* generation::tgff(generation_context &cntxt,int lower_bound,int max_od,int max_id)
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
Graph* generation::erdos_gnm(generation_context &cntxt, vertices_size num_vertices, edges_size num_edges, bool do_dag) 
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
Graph*  generation::random_orders(generation_context &cntxt, int num_vertices, int num_pos)
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

};

