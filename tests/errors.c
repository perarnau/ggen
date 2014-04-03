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
#include "error.h"
#include <assert.h>

int test_gsl_handler(gsl_rng *r)
{
	unsigned long max, min, ask;
	assert(ggen_error_start_stack() == 0);
	max = gsl_rng_max(r);
	min = gsl_rng_min(r);
	ask = max - min + 10;
	GGEN_CHECK_GSL_DO(ask = gsl_rng_uniform_int(r,ask));
	return 0;
ggen_error_label:
	return GGEN_FAILURE;
}

int test_internal_errno(void)
{
	// all ggen methods should fail on incorrect arguments
	assert(ggen_error_start_stack() == 0);
	GGEN_CHECK_INTERNAL_ERRNO(ggen_analyze_longest_path(NULL) != NULL);
	return 0;
ggen_error_label:
	return GGEN_FAILURE;
}

int test_clean(void *p)
{
	GGEN_CHECK_INTERNAL(ggen_error_start_stack());
	GGEN_FINALLY(free,p);
	GGEN_CHECK_INTERNAL(GGEN_EINVAL);
	ggen_error_clean(1);
	return 0;
ggen_error_label:
	return GGEN_FAILURE;
}

int main(int argc,char** argv)
{
	igraph_vector_t vector;
	igraph_t *g; 
	void *p;
	gsl_rng *r;	

	assert(ggen_error_start_stack() == 0);

	r = gsl_rng_alloc(gsl_rng_mt19937);
	GGEN_CHECK_GSL_ERRNO(r != 0);
	GGEN_FINALLY(gsl_rng_free,r);

	GGEN_CHECK_IGRAPH(igraph_vector_init(&vector,10));
	GGEN_FINALLY(igraph_vector_destroy,&vector);

	g = ggen_generate_erdos_gnm(r,10,0);
	GGEN_CHECK_INTERNAL_ERRNO(g != 0);
	GGEN_FINALLY(igraph_destroy,g);
	GGEN_FINALLY(free,g);
	
	assert(test_internal_errno() == GGEN_FAILURE);
	/* this one is invalid until the full source has been converted */
	//assert(ggen_errno.ggen_error == GGEN_EINVAL);

	p = malloc(10);
	assert(p != NULL);
	assert(test_clean(p) == GGEN_FAILURE);
	assert(ggen_errno.ggen_error == GGEN_EINVAL);

	assert(test_gsl_handler(r) == GGEN_FAILURE);
	assert(ggen_errno.gsl_error == GSL_EINVAL);

	ggen_error_clean(1);
	return 0;
ggen_error_label:
	return GGEN_FAILURE;
}
