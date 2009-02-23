#ifndef DYNAMIC_PROPERTIES_H
#define DYNAMIC_PROPERTIES_H

#include <boost/config.hpp>
#include <boost/dynamic_property_map.hpp>

#include "ggen.hpp"

/* typedefs for vertex and edge properties manipulations */
typedef graph_traits<Graph>::vertex_descriptor Vertex;
typedef graph_traits<Graph>::edge_descriptor Edge;
typedef graph_traits<Graph>::vertex_iterator Vertex_iter; 
typedef graph_traits<Graph>::edge_iterator Edge_iter; 

// UGLY & BAD HACK : we need to create a map with edge_descriptor as Key but this
// type doesn't implement the required '<' operator.
// see http://www.nabble.com/BGL:-std::map<Edge,-int>-needs-the-<-operator-td4019596.html  for more details

struct cmp_edge :
	public std::binary_function< Edge, Edge, bool>
{
		bool operator()(const Edge& e1, const Edge& e2) const
		{
			return e1.get_property() < e2.get_property();
		}
}; 

typedef std::map< Vertex, std::string >  vertex_std_map;
typedef std::map< Edge, std::string, cmp_edge >  edge_std_map;

typedef boost::associative_property_map< vertex_std_map > vertex_assoc_map;
typedef boost::associative_property_map< edge_std_map > edge_assoc_map;

void add_property(boost::dynamic_properties& dp, Graph& g,const char *name = "node_id", bool vertex_or_edge = true /* true = vertex property */);

// to use with any dynamic_properties constructor for automatic creation of property_maps
std::auto_ptr<dynamic_property_map> create_property_map (const std::string&, const boost::any& key, const boost::any& value);

#endif
