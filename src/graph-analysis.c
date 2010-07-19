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

#include "ggen.h"
#include <stdlib.h>

igraph_vector_t * ggen_analyze_longest_path(igraph_t *g)
{
	igraph_vector_t topology;
	igraph_vector_t lengths;
	igraph_vector_t preds;
	igraph_vs_t vs;
	igraph_vit_t vit;
	igraph_vector_t *res = NULL;
	int err;
	unsigned long v,i,f,t;
	long maxv;
	if(g == NULL)
		return NULL;

	v = igraph_vcount(g);
	err = igraph_vector_init(&topology,v);
	if(err)	return NULL;

	err = igraph_vector_init(&lengths,v);
	if(err) goto error_il;

	err = igraph_vector_init(&preds,v);
	if(err) goto error_ip;

	res = malloc(sizeof(igraph_t));
	if(res == NULL) goto cleanup;

	err = igraph_vector_init(res,v);
	if(err) goto error_ir;

	// sort topologically the vertices
	err = igraph_topological_sorting(g,&topology,IGRAPH_OUT);
	if(err) goto error;
	// igraph is stupid, it returns 0 even if the graph isn't a dag
	if(igraph_vector_size(&topology) != v)
		goto error;

	// find the best path incomming from every node
	igraph_vector_null(&lengths);
	igraph_vector_fill(&preds,-1);
	maxv = -1;
	for(i = 0; i < v; i++)
	{
		f = VECTOR(topology)[i];
		err = igraph_vs_adj(&vs,f,IGRAPH_OUT);
		if(err) goto error;
		err = igraph_vit_create(g,vs,&vit);
		if(err)
		{
			igraph_vs_destroy(&vs);
			goto error;
		}

		for(vit; !IGRAPH_VIT_END(vit); IGRAPH_VIT_NEXT(vit))
		{
			t = IGRAPH_VIT_GET(vit);
			if(VECTOR(lengths)[t] < VECTOR(lengths)[f] + 1)
			{
				VECTOR(lengths)[t] = VECTOR(lengths)[f] +1;
				VECTOR(preds)[t] = f;
			}
			if(maxv == -1 || VECTOR(lengths)[t] > VECTOR(lengths)[maxv])
				maxv = t;
		}
		igraph_vs_destroy(&vs);
		igraph_vit_destroy(&vit);

	}
	// build the path, using preds and maxv
	f = 0;
	while(maxv != -1)
	{
		VECTOR(*res)[f++] = maxv;
		maxv = VECTOR(preds)[maxv];
	}

	// finish the path correctly, resizing and reversing the array
	err = igraph_vector_resize(res,f);
	if(err) goto error;

	err = igraph_vector_reverse(res);
	if(err) goto error;

	goto cleanup;
error:
	igraph_vector_destroy(res);
error_ir:
	free(res);
	res = NULL;
cleanup:
	igraph_vector_destroy(&preds);
error_ip:
	igraph_vector_destroy(&lengths);
error_il:
	igraph_vector_destroy(&topology);
	return res;
}
