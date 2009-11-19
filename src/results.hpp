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

#include "types.hpp"
#include "dynamic_properties.hpp"

using namespace boost;
using namespace std;
namespace ggen {

/**
 * @file This file defines helper classes to compute statistics on the fly when analysing a graph.
 */

/** A result dumper
 * This class defines the basic functionalities of all possible "result savers".
 */
class ggen_result {
	public:
		 /** Set the ostream to use for dump 
		  * @param out the ostream to use for dumping result */
		void set_stream(ostream *out);

		/** dumps the result 
		 * @param properties this is needed to acces names of vertices in most results*/
		virtual void dump(dynamic_properties *properties);
	protected:
		ostream *os;
};

/********************************************************************
 * Graph specializations
 *******************************************************************/

/** Specialized result, used to save a graph */
class ggen_result_graph : public ggen_result {
	public:
		/** save a graph 
		 * @param graph the graph to save
		 * @param properties the graph properties associated
		 */
		virtual void save(Graph *graph);
};

/** A stupid result saver
 * Saves the graph and dump it in dot format */
class ggen_rg_stupid : public ggen_result_graph {
	public:
		/** Creates an empty result */
		ggen_rg_stupid();

		/** Simple destructor */
		~ggen_rg_stupid();

		/** Saves the graph */
		void save(Graph *graph);

		/** Dumps the graph using graphviz format */
		void dump(dynamic_properties *properties);
	private:
		/** the graph saved */
		Graph *g;
};

/********************************************************************
 * Paths specializations
 *******************************************************************/

/** Specialized result, used to save a list of paths */
class ggen_result_paths : public ggen_result {
	public:
		/** saves a path
		 * @param path the path to save 
		 */
		virtual void save(std::list<Vertex> path);
};

/** A stupid implementation of ggen_result_paths
 * This class is the simplest implementation of ggen_result_paths.
 * It saves all paths and dumps them in a pseudo-DOT format.
 * */
class ggen_rp_stupid : public ggen_result_paths {
	public:
		/** Creates an empty result */
		ggen_rp_stupid();

		/** Simple destructor */
		~ggen_rp_stupid();

		/** saves a path
		 * @param path the path to save 
		 */
		void save(std::list<Vertex> path);

		/** Dumps all paths */
		void dump(dynamic_properties *properties);
	private:
		/** a list of all paths saved */
		std::list< std::list< Vertex> > *Lpaths;
};


/********************************************************************
 * Vertex map specializations
 *******************************************************************/

/** Specialized result, used to save a map (Vertex,string) */
class ggen_result_vmap : public ggen_result {
	public:
		/** add a key/value pair
		 * @param key the vertex key
		 * @param value the string value
		 */
		virtual void save(Vertex key, string value);
};

/** A stupid implementation of ggen_result_vmap 
 * Only saves all key/values pairs and dumps them as following:
 * key : value
 * */
class ggen_rvm_stupid : public ggen_result_vmap {
	public:
		/** Creates an empty result */
		ggen_rvm_stupid();

		/** Simple destructor */
		~ggen_rvm_stupid();

		/** add a key/value pair
		 * @param key the vertex key
		 * @param value the string value
		 */
		void save(Vertex key, string value);

		/** Dumps the map */
		void dump(dynamic_properties *properties);
	private:
		/** the map used for saving */
		std::map < Vertex, std::string > map;
};
}
