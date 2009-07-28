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
#include "dynamic_properties.hpp"

using namespace boost;
namespace ggen {

/** add_property(dp, g, *name, vertex_or_edge)
*
* @param dp :
* @param g :
* @param *name :
* @param vertex_or_edge :
* computes the list of all connected components. We consider the graph undirected...
*/
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
}
