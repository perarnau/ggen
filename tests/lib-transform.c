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

/*
 * This program tests all generation methods present in the ggen library
 * These tests aren't validation campaings testing every possible statistic
 * available on these methods.
 * We just call each method with "normal" parameters and check the don't return null
 */

#include "ggen.h"
#include <stdio.h>
#include <stdlib.h>

#define PROLOGUE \
	fprintf(stdout,"in %s\n",__func__);	\
	enum ggen_transform_t t;		\
	int err;				\
	igraph_t g;				\
	igraph_full_citation(&g,100,1);

#define EPILOGUE						\
	if(err)							\
	{							\
		fprintf(stderr,"%s:%d:transformation failed\n",__FILE__,__LINE__);	\
		exit(EXIT_FAILURE);				\
	}							\
	igraph_destroy(&g);					\

void test_addsource()
{
	PROLOGUE
	t = GGEN_TRANSFORM_SOURCE;
	err = ggen_transform_add(&g,t);
	EPILOGUE
}
void test_addsink()
{
	PROLOGUE
	t = GGEN_TRANSFORM_SINK;
	err = ggen_transform_add(&g,t);
	EPILOGUE
}
void test_delsource()
{
	PROLOGUE
	t = GGEN_TRANSFORM_SOURCE;
	err = ggen_transform_delete(&g,t);
	EPILOGUE
}
void test_delsink()
{
	PROLOGUE
	t = GGEN_TRANSFORM_SINK;
	err = ggen_transform_add(&g,t);
	EPILOGUE
}

int main(int argc,char** argv)
{
	igraph_i_set_attribute_table(&igraph_cattribute_table);
	// test each of the transformations
	test_addsource();
	test_addsink();
	test_delsource();
	test_delsink();


	// if none of them failed, then return ok
	return 0;
}
