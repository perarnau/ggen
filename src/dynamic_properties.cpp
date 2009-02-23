#include "dynamic_properties.hpp"

using namespace boost;

void add_property(dynamic_properties& dp, Graph& g,const char *name /*= "node_id"*/, bool vertex_or_edge)
{
	if(vertex_or_edge)
	{
		vertex_std_map* map = new  vertex_std_map();
		vertex_assoc_map* amap = new vertex_assoc_map(*map);
		dp.property(name,*amap);
	}
	else
	{
		edge_std_map* map = new  edge_std_map();
		edge_assoc_map* amap = new edge_assoc_map(*map);
		dp.property(name,*amap);
	}
}

std::auto_ptr<dynamic_property_map> create_property_map (const std::string&, const boost::any& key, const boost::any& value)
{
	if( key.type() == typeid(Vertex) )
	{
		vertex_std_map* mymap = new vertex_std_map(); // hint: leaky memory here!
		vertex_assoc_map property_map(*mymap);
		std::auto_ptr<boost::dynamic_property_map> pm(
			new boost::detail::dynamic_property_map_adaptor<vertex_assoc_map>(property_map));
		return pm;
	}
	else if ( key.type() == typeid(Edge) )
	{
		edge_std_map* mymap = new edge_std_map(); // hint: leaky memory here!
		edge_assoc_map property_map(*mymap);
		std::auto_ptr<boost::dynamic_property_map> pm(
			new boost::detail::dynamic_property_map_adaptor<edge_assoc_map>(property_map));
		return pm;
	}
	else
		return std::auto_ptr<dynamic_property_map> (0); //TODO error handling
}
