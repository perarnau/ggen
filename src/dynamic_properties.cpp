#include "dynamic_properties.hpp"

using namespace boost;

void create_default_vertex_property(dynamic_properties& dp, Graph& g,const char *name /*= "node_id"*/)
{
	typedef std::map< graph_traits<Graph>::vertex_descriptor, int>  user_map;
	typedef boost::associative_property_map< user_map > name_map;
	typedef graph_traits<Graph>::vertex_iterator vertex_iter;
	user_map *map = new  user_map();
	name_map * bmap = new name_map(*map);
	dp.property(name,*bmap);
	
	std::pair<vertex_iter, vertex_iter> vp;
	int i =0;
	for (vp = boost::vertices(g); vp.first != vp.second; ++vp.first)
	{
		put(name,dp,*vp.first,i++);
	}

}

