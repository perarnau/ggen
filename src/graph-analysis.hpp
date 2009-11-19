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
#include <boost/graph/properties.hpp>

#include "types.hpp"
#include "dynamic_properties.hpp"
#include "results.hpp"

using namespace boost;
using namespace std;
namespace ggen {

/** minimum_spanning_tree(g)
*
* @param g : an object of type Graph to save the generated graph and it should be empty when initialized
* @param properties : the properties associated with the graph
*/
void minimum_spanning_tree(ggen_result_graph &r, const Graph& g, dynamic_properties& properties);


/** out_degree(g)
*
* @param g : an object of type Graph to save the generated graph and it should be empty when initialized
* @param properties : the properties associated with the graph
*
* It just outputs the out_degree of each node
*/
void out_degree(ggen_result_vmap &r, const Graph& g, dynamic_properties& properties);

/** in_degree(g)
*
* @param g : an object of type Graph to save the generated graph and it should be empty when initialized
* @param properties : the properties associated with the graph
*
* It just outputs the in_degree of each node
*/
void in_degree(ggen_result_vmap &r, const Graph& g, dynamic_properties& properties);

/** nodes_per_layer(g)
*
* @param g : an object of type Graph to save the generated graph and it should be empty when initialized
* @param properties : the properties associated with the graph
*
* this metric doesn't have a nice name for now
* the idea is to compute, for each i the number of nodes at a maximum distance (longest path) i of the source
* that is a kind of "number of nodes per layer"
*
* THIS MIGHT NOT WORK PROPERLY WITH NOT FULLY CONNECTED GRAPHS !!
*/
void nodes_per_layer(const Graph& g, dynamic_properties& properties);

/** longest_path(g)
*
* @param g : an object of type Graph to save the generated graph and it should be empty when initialized
* @param properties : the properties associated with the graph
*
* computes the longuest path present in the graph, this without weights on nodes nor edges
*/
void longest_path(ggen_result_paths &r, const Graph& g, dynamic_properties& properties);

/** max_i_s_rec(g, *max, current, allowed)
*
* @param g : an object of type Graph to save the generated graph and it should be empty when initialized
* @param *max :
* @param current :
* @param allowed : 
*
* recursive function for the max_independent_set
*/

void max_i_s_rec(const Graph& g,std::set<Vertex> *max,std::set<Vertex> current,std::set<Vertex> allowed);

/** max_independent_set(g)
*
* @param g : an object of type Graph to save the generated graph and it should be empty when initialized
* @param properties : the properties associated with the graph
*
* stupid "powerset" algorithm to computes the maximum independent set of the graph
* recursively compute all possible independent sets and find the maximum one
*/

void max_independent_set(const Graph& g, dynamic_properties& properties);

/** strong_components(g)
*
* @param g : an object of type Graph to save the generated graph and it should be empty when initialized
* @param properties : the properties associated with the graph
*
* computes the list of all connected components. We consider the graph undirected...
*/

void strong_components(const Graph& g, dynamic_properties& properties);

/** 
 *
 * @param g : a graph to analyse
 * @param properties : the properties associated with the graph
 *
 * displays the list of all paths of maximum length (ending by a sink) in the graph
 */

void maximal_paths(ggen_result_paths &r, const Graph& g, dynamic_properties& properties);

}
