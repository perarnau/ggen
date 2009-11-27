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

#include <boost/config.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/strong_components.hpp>
#include <boost/graph/topological_sort.hpp>

#include "types.hpp"
#include "dynamic_properties.hpp"
#include "graph-analysis.hpp"

using namespace boost;
using namespace std;

namespace ggen {

////////////////////////////////////////////////////////////////////////////////
// Analysis Function
////////////////////////////////////////////////////////////////////////////////

void minimum_spanning_tree(ggen_result_graph *r, const Graph& g, dynamic_properties& properties)
{
	Vertex source = *vertices(g).first;
	// We need a map to store the spanning tree
	std::map< Vertex, Vertex> vvmap;
	boost::associative_property_map< std::map< Vertex, Vertex> > predmap(vvmap);

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

	// Find a source of this graph
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
	prim_minimum_spanning_tree(g,predmap,root_vertex(source).weight_map(weightmap).vertex_index_map(indexmap).visitor(default_dijkstra_visitor()));

	// Build the mst has a graph and initialize correctly its properties
	Graph mst;
	dynamic_properties p(&create_property_map);
	std::map< Vertex, Vertex > o2n;
	for (vp = boost::vertices(g); vp.first != vp.second; ++vp.first)
	{
		o2n[*vp.first] = add_vertex(mst);
		std::string name = get("node_id",properties,*vp.first);
		put("node_id",p,o2n[*vp.first],name);
	}
	for (vp = boost::vertices(g); vp.first != vp.second; ++vp.first)
	{
		if (predmap[*vp.first] != *vp.first)
			add_edge(o2n[predmap[*vp.first]],o2n[*vp.first],mst);
	}
	r->save(&mst);
	r->dump(&p);
}

void out_degree(ggen_result_vmap *r, const Graph& g, dynamic_properties& properties)
{
	std::pair<Vertex_iter, Vertex_iter> vp;
	for(vp = boost::vertices(g); vp.first != vp.second; ++vp.first)
	{
		r->save(*vp.first,lexical_cast<std::string>(out_degree(*vp.first,g)));
	}
	r->dump(&properties);
}

void in_degree(ggen_result_vmap *r, const Graph& g, dynamic_properties& properties)
{
	std::pair<Vertex_iter, Vertex_iter> vp;
	for(vp = boost::vertices(g); vp.first != vp.second; ++vp.first)
	{
		r->save(*vp.first,lexical_cast<std::string>(in_degree(*vp.first,g)));
	}
	r->dump(&properties);
}

void nodes_per_layer(ggen_result_vsets *r, const Graph& g, dynamic_properties& properties)
{
	std::set< Vertex> src;
	std::pair< Vertex_iter, Vertex_iter> vp;

	// find all sources
	for(vp = vertices(g); vp.first != vp.second; ++vp.first)
	{
		if(in_degree(*vp.first,g) == 0)
		{
			src.insert(*vp.first);
		}
	}

	// now we want to compute our metric
	// run through the nodes beginning by the source and list its successors
	// iff all the predecessors have been visited
	std::set< Vertex > cur,next;
	std::set< Vertex > visited;
	std::set<Vertex>::iterator it;
	unsigned int i = 0;
	//std::cout << "Nodes Per Layer:" << std::endl;
	cur = src;
	while( cur.size() != 0 )
	{
		//std::cout << "Layer " << i++ << ":";
		std::set< Vertex > layer;
		for(it = cur.begin(); it != cur.end(); it++)
		{
			// we visited the node
			//std::cout << " " << get("node_id",properties,*it);
			layer.insert(*it);
			visited.insert(*it);

			// look for its successors
			std::pair< Out_edge_iter, Out_edge_iter> ep;
			for(ep = out_edges(*it,g); ep.first != ep.second; *ep.first++)
			{
				Vertex suc = boost::target(*ep.first,g);
				// have all the predecessors of this node been visited ?
				bool v = true;
				std::pair< In_edge_iter, In_edge_iter> ip;
				for(ip = in_edges(suc,g); ip.first != ip.second; *ip.first++)
				{
					if(visited.count(boost::source(*ip.first,g)) == 0)
					{
						if(cur.count(boost::source(*ip.first,g)) == 0)
						{
							v = false;
							break;
						}
					}
				}
				if(v)
					next.insert(suc);
			}
		}
		r->save(layer);
		cur = next;
		next.clear();
	}
	r->dump(&properties);
}

void longest_path(ggen_result_paths *r, const Graph& g, dynamic_properties& properties)
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
	boost::topological_sort(g,il,vertex_index_map(indexmap));

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
	Vertex s;
	std::list< Vertex > path;
	path.push_front(maxv);
	while(pmap.find(maxv) != pmap.end())
	{
		s = pmap[maxv];
		path.push_front(s);
		maxv = s;
	}
	r->save(path);
	r->dump(&properties);
}

/** max_i_s_rec(g, *max, current, allowed)
*
* @param g : an object of type Graph to save the generated graph and it should be empty when initialized
* @param *max : the current maximal set
* @param current : the current vertex evaluated
* @param allowed : the vertices still allowed to include in the maximal set
*
* recursive function for the max_independent_set
*/
void max_i_s_rec(const Graph& g,std::set<Vertex> *max,std::set<Vertex> current,std::set<Vertex> allowed)
{
	// optimization, if there is not enough available nodes for current
	// to grow bigger than current max, we can stop
	if(current.size() + allowed.size() <= max->size())
		return;

	// stop condition, we have marked all the vertices of the graph
	if(allowed.empty())
	{
		if(max->size() < current.size())
		{
			delete max;
			max = new std::set<Vertex>(current);
		}
	}
	else // standard case, we choose either to insert a new vertex or not
	{
		// choose a vertex, one in allowed
		std::set<Vertex>::iterator it = allowed.begin();
		Vertex c = *it;
		// remove from allowed
		allowed.erase(c);

		// first case, we add the vertex, and remove all its adjacent vertices from the graph as they do not comply with
		// the independent set property
		std::set<Vertex> c2(current);
		c2.insert(c);

		std::set<Vertex> a2(allowed);
		// find the adjacent vertices and do not allow them, we consider the graph undirected here
		std::pair<In_edge_iter, In_edge_iter> ip;
		for(ip = in_edges(c,g); ip.first != ip.second; ++ip.first)
		{
			Vertex v = source(*ip.first,g);
			a2.erase(v);
		}
		std::pair<Out_edge_iter, Out_edge_iter> op;
		for(op = out_edges(c,g); op.first != op.second; ++op.first)
		{
			Vertex v = target(*op.first,g);
			a2.erase(v);
		}
		max_i_s_rec(g,max,c2,a2);

		// second recursion, we don't add the vertex to current set
		max_i_s_rec(g,max,current,allowed);
	}
}

void max_independent_set(ggen_result_vsets *r, const Graph& g, dynamic_properties& properties)
{
	// the list of sets
	std::set< Vertex > *max = new  std::set< Vertex >();
	// the current list computed
	std::set< Vertex > empty;
	// the set of allowed vertices, these can be added to a set
	std::pair<Vertex_iter,Vertex_iter> vp = vertices(g);
	std::set<Vertex> a(vp.first,vp.second);
	// launch the recursion
	max_i_s_rec(g,max,empty,a);

	// save the maximum independant set
	r->save(*max);
	r->dump(&properties);
}

void strong_components(ggen_result_vsets *r, const Graph& g, dynamic_properties& properties)
{
	// create the component map
	std::map< Vertex, int > com;
	boost:associative_property_map< std::map< Vertex, int > > comp(com);

	// the index map
	int i = 0;
	std::map < Vertex, int > imap;
	boost::associative_property_map< std::map< Vertex, int> > indexmap(imap);

	// Update map
	std::pair<Vertex_iter, Vertex_iter> vp;
	for (vp = boost::vertices(g); vp.first != vp.second; ++vp.first)
	{
		imap.insert(make_pair(*vp.first,i++));
	}

	int num = boost::strong_components(g, comp, vertex_index_map(indexmap));

	// create the sets from the map
	//std::cout << "Total number of components: " << num << std::endl;
	std::map< Vertex, int>::iterator it;
	std::map< int, std::set< Vertex > > sets;
	for(it = com.begin(); it != com.end() ; ++it)
	{
		sets[it->second].insert(it->first);
	}
	// save the sets
	std::map< int, std::set< Vertex > >::iterator sit;
	for(sit = sets.begin(); sit!= sets.end(); sit++)
	{
		r->save(sit->second);
	}
	r->dump(&properties);
}

/**
 * @param r : the ggen_result used to save and dump the result
 * @param g : the graph analysed
 * @param properties : the properties associated with the graph
 * @param current_path : the current path already visited (current_node excluded)
 * @param current_node : the current node visited
 * internal recursive function to compute the maximal paths
 */
static void maximal_paths_internal(ggen_result_paths *r, const Graph& g, dynamic_properties& properties, std::list< Vertex > current_path, Vertex current_node)
{
	// if node is sink, the path is finished: save it
	// else call recursively on each child
	if(boost::out_degree(current_node,g) == 0)
	{
		current_path.push_back(current_node);
		r->save(current_path);
	}
	else
	{
		// add current_node to path
		current_path.push_back(current_node);
		// for each child make a recursive call
		std::pair< Out_edge_iter, Out_edge_iter> ep;
		for(ep=boost::out_edges(current_node,g);ep.first != ep.second; ++ep.first)
		{
			Vertex v = boost::target(*ep.first,g);
			maximal_paths_internal(r,g,properties,current_path,v);
		}
	}
}

/**
 * displays the list of all paths of maximum length (ending by a sink) in the graph
 */

void maximal_paths(ggen_result_paths *r, const Graph& g, dynamic_properties& properties)
{
	// init path
	std::list<Vertex> path;

	// find sources to launch the recursion
	std::pair< Vertex_iter, Vertex_iter > vp;
	for(vp=boost::vertices(g); vp.first != vp.second; ++vp.first)
	{
		if(boost::in_degree(*vp.first,g) == 0)
		{
			maximal_paths_internal(r,g,properties,path,*vp.first);
		}
	}
	r->dump(&properties);
}

}
