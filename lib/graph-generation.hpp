/** Copyright Swann Perarnau 2009
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
*
* GGen is a random graph generator :
* it provides means to generate a graph following a
* collection of methods found in the litterature.
*
* This is a research project founded by the MOAIS Team,
* INRIA, Grenoble Universities.
*/

#ifndef GRAPH_GENERATION_HPP
#define GRAPH_GENERATION_HPP

#include "types.hpp"
#include "random.hpp"

using namespace boost;
namespace ggen {

/** This class contains all the context (variables and other stuff)
 * needed by the generation methods.
 */
class generation_context {
	public:
		/** Creates an empty context 
		 * @return an empty context that should be populated by set* methods*/
		generation_context();

		/** Context destructor, will destroy any object in it*/
		~generation_context();

		/** Sets the rng for this context
		 * @param r : the Random Number Generator to be used by generation methods */
		void set_rng(ggen_rng* r);

		/** Retreives the rng of this context
		 * @return the Random Number Generator of this context, NULL if the context is empty */
		ggen_rng* get_rng();
	private:
		/** The Random Number Generator to be used by the generation methods */
		ggen_rng* rng;
};	

/** This class enclose all generation methods available in GGen.
 * Every method returns a Graph and takes a least a Graph_generation_context.
 * Two utils function are also provided, including the very useful adjacency matrix translation.
 * @see Graph_generation_context
 */
class generation {
	public:
	
	/** translate_matrix_to_a_graph(g, matrix, num_vertices)
	 *
	 * A method to convert an adjacency matrix to an object of type Graph.
	 *
	 * @return an object of type Graph containing the generated graph
	 * @param matrix : an adjacency matrix with matrix[i][j] true if there is an edge connecting the vertices 'i' & 'j' otherwise false.
	 * @param num_vertices : the number of vertices in the graph 
	 *
	 */
	 static Graph* translate_matrix_to_a_graph(bool **matrix, vertices_size num_vertices);

	/** Erdos-Renyi : G(n,p)
	 *
	 * One of the simplest way of generating a graph
	 *
	 * Supports :
	 *
	 * -- dag option
	 *
	 * @return an object of type Graph to containing the generated graph
	 * @param cntxt : the context used for the generation
	 * @param num_vertices : the number of vertices in the graph
	 * @param p : probability with which an edge(i,j) will be formed.
	 * @param do_dag : a boolean to determine whether the graph is a directed acyclic graph or not.
	 *
	 * This method first of all generates an adjacency matrix of the type boolean of size equal to the number of vertices. Then a coin is flipped for every 
	 * (i,j) to determine whether an edge be inserted between the vertices 'i' and 'j' or not.If do_dag is true then only the upper triangle of the adjacency  
	 * matrix is iterated otherwise the complete adjacency matrix.Finally, the adjacency matrix is translated to a graph object with the help of the function  
	 * translate_matrix_to_a_graph.
	 */
	static Graph* erdos_gnp(generation_context &cntxt, vertices_size num_vertices, double p, bool do_dag);

	/** Layer-by-Layer Method: gg_layer_by_layer(g, num_vertices, p, do_dag, layer_num_vertex)
	 *
	 * Using coin flipping to connect the layers
	 *
	 * Supports :
	 *
	 * -- dag option
	 *
	 * @return an object of type Graph to containing the generated graph.
	 * @param cntxt : the context to use for this generation
	 * @param num_vertices : number of vertices in the graph
	 * @param p : probability with which an edge will be inserted between two layers
	 * @param do_dag : a boolean to determine whether the graph is a directed acyclic graph or not
	 * @param layer_num_vertex : an array containing index of the layer in which a vertex is positioned.
	 * 
	 * In this method, two vertices lying in distinct layers are chosen and an edge is inserted between them depending upon the outcome of a bernouilli trial.
	 * No edge is inserted between the two vertices lying in the same layer.
	 */
	static Graph*  layer_by_layer(generation_context &cntxt, vertices_size num_vertices, double p, bool do_dag,std::vector<int> layer_num_vertex);

	/** The method 'layer_allocation(num_layers, num_vertices)' returns an array containing layer indices for all the vertices. 
	 * This array is required for the random graph generation method "Layer-by-Layer".
	 *
	 * @param cntxt : the context to use.
	 * @param num_layers : number of layers in the graph
	 * @param num_vertices : number of vertices in the graph
	 * 
	 * A random number between 1 and 'num_layers' is generated and is assigned to each vertex.
	 *
	 */
	static std::vector <int> layer_allocation(generation_context &cntxt,unsigned long int num_layers,vertices_size num_vertices);

	/** Task Graphs for free: gg_tgff(g, lower_bound, max_od, max_id)
	 *
	 * @return an object of type Graph to containing the generated graph.
	 * @param cntxt : the context to use for this generation.
	 * @param lower_bound : a lower bound on the number of vertices ensuring that atleast these many vertices should be present in the generated graph
	 * @param max_od : maximum out degree constraint on each node
	 * @param max_id : maximum in degree constraint on each node
	 *
	 * The Task Graphs for Free method carries out graph generation by iteratively incorporating two steps 'fan-out' and 'fan-in'.First of all, a starting node  
	 * i.e. 0th node is created. A bernoulli trial is conducted to determine on each iteration whether it should be a fan-out step or a fan-in step.
	 * If it is a fan-out step then available out degrees for all the existing vertices are calculated. The maximum of these all the available outdegrees is 
	 * determined.And vertices having available out degree equal to this maximum are stored in an array named available_vertices.Then, a random number is chosen 
	 * between 1 and this maximum.New nodes as equal to the generated random number are introduced and each node is connected to an unique vertex in the array 
	 * available_vertices.
	 * If it a fan-in step then a new node is introduced. All the existing vertices with available out degree greater than zero are stored into an array 
	 * and maximum of this array is calculated.Now all the vertices having available out degree equal to this maximum are stored into a separate array.The size 
	 * of the newly formed array is said to be the cardinality.If cardinality exceeds the maximum indegree constraint then cardinality is set to the maximum 
	 * in-degree constraint.A random number is generated between 1 and this cardinality and as many number of the vertices are chosen randomly from the newly 
	 * formed array and each vertex just chosen is connected to the new node introuduced in the starting of the fan-in step.
	 */
	static Graph* tgff(generation_context &cntxt,int lower_bound,int max_od,int max_id);

	/** Erdos-Renyi : G(n,M)
	 *
	 * One of the simplest way of generating a graph
	 *
	 * Supports :
	 *
	 * -- dag option
	 *
	 * @return the generated Graph.
	 * @param cntxt : the context to use.
	 * @param num_vertices : the number of vertices in the graph
	 * @param num_edges : the number of edges in the graph
	 * @param do_dag : a boolean to determine whether the graph is a directed acyclic graph or not.
	 *
	 * In this method, an edge is inserted between the two vertices 'i' and 'j' selected randomly.And the selection of these two vertices is iteratively 
	 * carried out unless the total number of edges become equal to the wanted number of edges.
	 */
	static Graph* erdos_gnm(generation_context &cntxt, vertices_size num_vertices, edges_size num_edges, bool do_dag);


	/** Random Orders Method : 
	 *
	 * @return the generated Graph.
	 * @param cntxt : the context to use.
	 * @param num_vertices : the number of vertices in the graph
	 * @param num_pos : the number of partially ordered sets which are permutations of the orders of the vertices
	 *
	 * In this method, an array poset with no. of rows equal to the num_pos and no. of columns equal to num_vertices is created.In this array, each row of it
	 * represents a permutation of the vertices.With the help of the function 'shuffle' each row of this array is shuffled. If the vertex 'i' appears before the 
	 * vertex 'j' in every permutation, an edge(i, j) is inserted.And, finally, the adjacency matrix is translated to a graph object with the help of the 
	 * translate_matrix_to_a_graph.
	 */
	static Graph* random_orders(generation_context &cntxt, int num_vertices, int num_pos);

};
}

#endif
