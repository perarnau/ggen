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

/* This program is an usage example of the longest path method of ggen.
 * It also works as a unit test.
 */

#include "ggen.h"
#include <assert.h>

int main(int argc,char** argv)
{
	igraph_vector_t *lp;
	igraph_t g;

	// all ggen methods should fail on incorrect arguments
	assert(ggen_analyze_longest_path(NULL) == NULL);

	// an empty graph as a longest path of zero
	igraph_empty(&g,10,1);
	lp = ggen_analyze_longest_path(&g);
	assert(lp != NULL);
	assert(igraph_vector_size(lp) == 0);
	igraph_destroy(&g);
	igraph_vector_destroy(lp);
	free((void *)lp);

	// a full dag as a longest path of containing all vertices
	igraph_full_citation(&g,10,1);
	lp = ggen_analyze_longest_path(&g);
	assert(lp != NULL);
	assert(igraph_vector_size(lp) == 10);
	igraph_destroy(&g);
	igraph_vector_destroy(lp);
	free((void *)lp);

	// a simple graph should work too
	// graph is 0 -> 1 -> 2 and 0 -> 2
	igraph_small(&g,10,1,0,1,1,2,0,2,-1);
	lp = ggen_analyze_longest_path(&g);
	assert(lp != NULL);
	assert(igraph_vector_size(lp) == 3);
	igraph_destroy(&g);
	igraph_vector_destroy(lp);
	free((void *)lp);

	return 0;
}
