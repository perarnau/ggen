/* Copyright Swann Perarnau 2009
 *
 *   contributor(s) :
 *
 *   contact : Swann.Perarnau@imag.fr
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

 /* This program tests the transform_delete method of ggen
 */

#include "ggen.h"
#include <assert.h>

int main(int argc,char** argv)
{
	igraph_t g;
	enum ggen_transform_t t;

	// all ggen methods should fail on incorrect arguments
	assert(ggen_transform_delete(NULL,GGEN_TRANSFORM_SINK));

	// graph is
	//		----------v
	//		0--->1--->2
	//		3----^
	igraph_small(&g,4,1,0,1,0,2,1,2,3,1,-1);
	assert(ggen_transform_delete(&g,GGEN_TRANSFORM_SOURCE)== 0);
	// check the number of edges deleted from the graph
	assert(igraph_ecount(&g) == 1);
	assert(igraph_vcount(&g) == 2);
	igraph_destroy(&g);

	// graph is
	//		----------v
	//		0--->1--->2
	//		     ---->3
	igraph_small(&g,4,1,0,1,0,2,1,2,1,3,-1);
	assert(ggen_transform_delete(&g,GGEN_TRANSFORM_SINK) == 0);
	// check the number of edges added to the graph
	assert(igraph_ecount(&g) == 1);
	assert(igraph_vcount(&g) == 2);
	igraph_destroy(&g);

	return 0;
}
