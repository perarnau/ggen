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

/* This file is an usage exemple of the gnm generation method. It also serves
 * as a test file to ensure the method work correctly
 */

#include "ggen.h"
#include <assert.h>

int main()
{
	igraph_t *g;
	gsl_rng *r;

	r = gsl_rng_alloc(gsl_rng_mt19937);
	assert(r != NULL);

	// all ggen methods return NULL on invalid parameters
	assert(ggen_generate_erdos_gnm(NULL,10,10) == NULL);
	assert(ggen_generate_erdos_gnm(r,10,100) == NULL);

	// if m equals zero the graph should have zero edges
	g = ggen_generate_erdos_gnm(r,10,0);
	assert(g != NULL);
	assert(igraph_ecount(g) == 0);
	igraph_destroy(g);
	free((void *)g);

	// if m equals n*(n-1)/2 the graph should have n*(n-1)/2 edges
	g = ggen_generate_erdos_gnm(r,10,45);
	assert(g != NULL);
	assert(igraph_ecount(g) == 45);
	igraph_destroy(g);
	free((void *)g);

	// a "normal" call should word too
	g = ggen_generate_erdos_gnm(r,10,20);
	assert(g != NULL);
	assert(igraph_ecount(g) == 20);
	igraph_destroy(g);
	free((void *)g);

	gsl_rng_free(r);
	return 0;
}
