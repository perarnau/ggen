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
#ifndef DYNAMIC_PROPERTIES_H
#define DYNAMIC_PROPERTIES_H

#include <boost/config.hpp>
#include <boost/dynamic_property_map.hpp>

#include "types.hpp"
namespace ggen {

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

}

#endif
