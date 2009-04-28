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

#ifndef GGEN_H
#define GGEN_H

/* We use extensively the BOOST library for 
 * handling output, program options and random generators
 */
#include <boost/config.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/dynamic_property_map.hpp>

using namespace boost;

/* This is the definition of the graph struture
 * According to boost that means :
 *	* The graph is an adjacency list
 *	* The vertices are managed as a std::Vector
 *	* The edges are managed as std::set to enforce no parallel edges
 *	* The graph is bidirectional
 *	* We don't have any additional properties on vertices or edges
 */
typedef adjacency_list < listS, listS, bidirectionalS, dynamic_properties, dynamic_properties > Graph;

/* typedefs for vertex and edge properties manipulations */
typedef graph_traits<Graph>::vertex_descriptor Vertex;
typedef graph_traits<Graph>::edge_descriptor Edge;
typedef graph_traits<Graph>::vertex_iterator Vertex_iter; 
typedef graph_traits<Graph>::edge_iterator Edge_iter; 
typedef graph_traits<Graph>::in_edge_iterator In_edge_iter; 
typedef graph_traits<Graph>::out_edge_iterator Out_edge_iter; 


/* Debug macro, usefull for tracing program execution */
// TODO: debug asserts, use it

#ifdef DEBUG
#define dbg(format,...) fprintf(stderr, "[file %s, line %d] " format, __FILE__, __LINE__ , __VA_ARGS__)
#else
#define dbg(format,...) do { } while(0)
#endif
#endif
