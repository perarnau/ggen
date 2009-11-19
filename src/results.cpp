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
#include <boost/graph/properties.hpp>
#include <boost/graph/graphviz.hpp>

#include "types.hpp"
#include "dynamic_properties.hpp"
#include "results.hpp"

using namespace boost;
using namespace std;
namespace ggen {

void ggen_result::set_stream(ostream *out) {
	os = out;
}

/* Graph result */

ggen_rg_stupid::ggen_rg_stupid() {
	g = NULL;
}

ggen_rg_stupid::~ggen_rg_stupid() {
	if(g != NULL)
		delete g;
}

void ggen_rg_stupid::save(Graph *graph) {
	g = graph;
}

void ggen_rg_stupid::dump(dynamic_properties *properties) {
	if(g != NULL && os != NULL)
		write_graphviz(*os,*g,*properties);
}

/* Paths result */

ggen_rp_stupid::ggen_rp_stupid() {
	Lpaths = new std::list< std::list < Vertex> >();
}

ggen_rp_stupid::~ggen_rp_stupid() {
	delete Lpaths;
}

void ggen_rp_stupid::save(std::list<Vertex> path) {
	Lpaths->push_back(path);
}

void ggen_rp_stupid::dump(dynamic_properties *properties) {
	// output each list independently
	std::list< std::list< Vertex> >::iterator it;
	for(it=Lpaths->begin();it != Lpaths->end(); it++) {
		std::list<Vertex> p = *it;
		std::list<Vertex>::iterator vit;
		vit = p.begin();
		if(vit == p.end())
			continue;

		Vertex cur = *vit;
		*os << get("node_id",*properties,cur);
		vit++;
		while(vit != p.end()) {
			*os << " -> " << get("node_id",*properties,*vit);
			vit++;
		}
		*os << std::endl;
	}
}

/* Vertex map result */
ggen_rvm_stupid::ggen_rvm_stupid() {
}

ggen_rvm_stupid::~ggen_rvm_stupid() {
}

void ggen_rvm_stupid::save(Vertex key, string value) {
	map[key] = value;
}

void ggen_rvm_stupid::dump(dynamic_properties *properties) {
	std::map< Vertex, std::string >::iterator it;
	for(it = map.begin(); it!= map.end(); it++) {
		*os << get("node_id",*properties,it->first) << " : " << it->second << std::endl;
	}
}

/* Vertex sets result */
ggen_rvs_stupid::ggen_rvs_stupid() {
}

ggen_rvs_stupid::~ggen_rvs_stupid() {
}

void ggen_rvs_stupid::save(std::set < Vertex > set) {
	sets.push_back(set);
}

void ggen_rvs_stupid::dump(dynamic_properties *properties) {
	std::list < std::set < Vertex > >::iterator it;
	// iterate over the list
	for(it = sets.begin(); it != sets.end(); it++) {
		// display each set
		// some complications due to proper comma display
		std::set < Vertex > s = *it;
		std::set< Vertex >::iterator sit;
		sit = s.begin();
		if(sit == s.end())
			continue;

		*os << get("node_id",*properties,*sit);
		sit++;
		while(sit != s.end()) {
			*os << ", " << get("node_id",*properties,*sit);
			sit++;
		}
		*os << std::endl;
	}
}

}

