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

/* This is currently an Hopcroft-Karp implementation.
 * See http://en.wikipedia.org/wiki/Hopcroft_Karp
 * Warning: this code assume the bipartite graph is balanced :
 * vcount/2 vertices are on the left, and the same number on the
 * right.
 */
#include <igraph/igraph.h>
#include "error.h"

static int bfs(igraph_t *g, igraph_vector_t *pair, igraph_vector_t *layer, int *ret)
{
	unsigned long i,j,vg;
	igraph_dqueue_t q;
	int err, nil_value;
	igraph_vit_t vit;
	igraph_vs_t vs;

	ggen_error_start_stack();
	vg = igraph_vcount(g);

	GGEN_CHECK_IGRAPH(igraph_dqueue_init(&q,vg));
	GGEN_FINALLY(igraph_dqueue_destroy,&q);

	for(i = 0; i < vg/2; i++)
	{
		if(VECTOR(*pair)[i] == (igraph_real_t)vg)
		{
			VECTOR(*layer)[i] = 0;
			igraph_dqueue_push(&q,(igraph_real_t)i);
		}
		else
			VECTOR(*layer)[i] = vg;
	}

	VECTOR(*layer)[vg] = vg;
	while(!igraph_dqueue_empty(&q))
	{
		i = (unsigned long)igraph_dqueue_pop(&q);
		if(i != vg)
		{
			GGEN_CHECK_IGRAPH(igraph_vs_adj(&vs,i,IGRAPH_ALL));
			GGEN_FINALLY(igraph_vs_destroy,&vs);

			GGEN_CHECK_IGRAPH(igraph_vit_create(g,vs,&vit));
			GGEN_FINALLY(igraph_vit_destroy,&vit);

			for(IGRAPH_VIT_RESET(vit); !IGRAPH_VIT_END(vit); IGRAPH_VIT_NEXT(vit))
			{
				j = (unsigned long)IGRAPH_VIT_GET(vit);
				j = (unsigned long)VECTOR(*pair)[j];
				if(VECTOR(*layer)[j] == (igraph_real_t)vg)
				{
					VECTOR(*layer)[j] = VECTOR(*layer)[i] +1;
					igraph_dqueue_push(&q,(igraph_real_t)j);
				}
			}
			// clean vs and vit
			ggen_error_pop(2);
		}
	}
	if(VECTOR(*layer)[vg] == (igraph_real_t)vg)
		*ret = 0;
	else
		*ret = 1;

	ggen_error_clean(1);
	return GGEN_SUCCESS;
ggen_error_label:
	return GGEN_FAILURE;
}

static int dfs(igraph_t *g, unsigned long i, igraph_vector_t *pair, igraph_vector_t *layer, int *ret)
{
	igraph_vit_t vit;
	igraph_vs_t vs;
	unsigned long j,vg,p;
	int val;

	ggen_error_start_stack();
	vg = igraph_vcount(g);
	if(i == vg)
	{
		*ret =1;
		goto success;
	}

	GGEN_CHECK_IGRAPH(igraph_vs_adj(&vs,i,IGRAPH_ALL));
	GGEN_FINALLY(igraph_vs_destroy,&vs);

	GGEN_CHECK_IGRAPH(igraph_vit_create(g,vs,&vit));
	GGEN_FINALLY(igraph_vit_destroy,&vit);

	for(IGRAPH_VIT_RESET(vit); !IGRAPH_VIT_END(vit); IGRAPH_VIT_NEXT(vit))
	{
		j = IGRAPH_VIT_GET(vit);
		p = (unsigned long)VECTOR(*pair)[j];
		if(VECTOR(*layer)[p] == (igraph_real_t) (VECTOR(*layer)[i] +1))
		{
			GGEN_CHECK_INTERNAL(dfs(g,p,pair,layer,&val));
			if(val)
			{
				VECTOR(*pair)[j] = i;
				VECTOR(*pair)[i] = j;
				*ret = 1;
				goto success;
			}
		}
	}
	VECTOR(*layer)[i] = (igraph_real_t)vg;
	*ret = 0;
success:
	ggen_error_clean(1);
	return GGEN_SUCCESS;
ggen_error_label:
	return GGEN_FAILURE;
}


int bipartite_maximum_matching(igraph_t *g, igraph_vector_t *res)
{
	igraph_vector_t pair, layer;
	int nil_value;
	unsigned long i,vg;
	igraph_integer_t eid;
	int bfscontinue, dfsret;

	ggen_error_start_stack();
	if(g == NULL || res == NULL)
		GGEN_SET_ERRNO(GGEN_EINVAL);

	vg = (unsigned long)igraph_vcount(g);
	if(vg%2)
		GGEN_SET_ERRNO(GGEN_EINVAL);

	GGEN_CHECK_IGRAPH(igraph_vector_init(&pair,vg+1));
	GGEN_FINALLY(igraph_vector_destroy,&pair);
	igraph_vector_fill(&pair,(igraph_real_t)vg);

	GGEN_CHECK_IGRAPH(igraph_vector_init(&layer,vg+1));
	GGEN_FINALLY(igraph_vector_destroy,&layer);

	GGEN_CHECK_INTERNAL(bfs(g,&pair,&layer,&bfscontinue));
	while(bfscontinue)
	{
		for(i = 0; i < vg; i++)
		{
			if(VECTOR(pair)[i] == (igraph_real_t)vg)
			{
				GGEN_CHECK_INTERNAL(dfs(g,i,&pair,&layer,&dfsret));
			}
		}
		GGEN_CHECK_INTERNAL(bfs(g,&pair,&layer,&bfscontinue));
	}
	/* convert pair to matching */
	for(i = 0; i < vg/2; i++)
	{
		if(VECTOR(pair)[i] != (igraph_real_t)vg)
		{
			igraph_get_eid(g,&eid,i,VECTOR(pair)[i],0,0);
			igraph_vector_push_back(res,eid);
		}
	}
	ggen_error_clean(1);
	return GGEN_SUCCESS;
ggen_error_label:
	return GGEN_FAILURE;
}
