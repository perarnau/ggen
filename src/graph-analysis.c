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
#include "vector_utils.h"
#include "bipartite-matching.h"
#include "tree-lowest-common-ancestor.h"

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

	res = malloc(sizeof(igraph_vector_t));
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

/* An antichain is defined as a set of vertices so that
 * no path exist between any couple of them.
 * For example, in the graph :
 *               1
 *             /   \
 *            2     3
 *             \   /
 *               4
 * with edges going downward, the sets {1}, {2,3} and {4} are
 * antichains, but NOT {1,2}, {1,3} or {2,4}...
 *
 * This implementation finds the antichain of maximal size
 * by solving an equivalent problem : vertex cover on a bipartite
 * graph. This is solved by computing a maximum matching and
 * converting it to a vertex cover.
 *
 * See the following links to understand what's going on :
 * https://en.wikipedia.org/wiki/Dilworth%27s_theorem
 * http://en.wikipedia.org/wiki/K%C3%B6nig%27s_theorem_%28graph_theory%29
 * http://en.wikipedia.org/wiki/Hopcroft%E2%80%93Karp_algorithm
 *
 */

igraph_vector_t * ggen_analyze_longest_antichain(igraph_t *g)
{
	/* The following steps are implemented :
	 *  - Convert our DAG to a specific bipartite graph B
	 *  - solve maximum matching on B
	 *  - conver maximum matching to min vectex cover
	 *  - convert min vertex cover to antichain on G
	 */
	int err;
	unsigned long i,vg,found,added;
	igraph_t b,gstar;
	igraph_vector_t edges,*res = NULL;
	igraph_vector_t c,s,t,todo,n,next,l,r;
	igraph_eit_t eit;
	igraph_es_t es;
	igraph_integer_t from,to;
	igraph_vit_t vit;
	igraph_vs_t vs;
	igraph_real_t value;

	if(g == NULL)
		return NULL;

	/* before creating the bipartite graph, we need all relations
	 * between any two vertices : the transitive closure of g */
	err = igraph_copy(&gstar,g);
	if(err) return NULL;

	err = ggen_transform_transitive_closure(&gstar);
	if(err) goto error;

	/* Bipartite convertion : let G = (S,C),
	 * we build B = (U,V,E) with
	 *	- U = V = S (each vertex is present twice)
	 *	- (u,v) \in E iff :
	 *		- u \in U
	 *		- v \in V
	 *		- u < v in C (warning, this means that we take
	 *		transitive closure into account, not just the
	 *		original edges)
	 * We will also need two additional nodes further in the code.
	 */
	vg = igraph_vcount(g);
	err = igraph_empty(&b,vg*2,1);
	if(err) goto error;

	/* id and id+vg will be a vertex in U and its copy in V,
	 * iterate over gstar edges to create edges in b
	 */
	err = igraph_vector_init(&edges,igraph_ecount(&gstar));
	if(err) goto d_b;
	igraph_vector_clear(&edges);

	err = igraph_eit_create(&gstar,igraph_ess_all(IGRAPH_EDGEORDER_ID),&eit);
	if(err) goto d_edges;

	for(IGRAPH_EIT_RESET(eit); !IGRAPH_EIT_END(eit); IGRAPH_EIT_NEXT(eit))
	{
		err = igraph_edge(&gstar,IGRAPH_EIT_GET(eit),&from,&to);
		if(err)
		{
			igraph_eit_destroy(&eit);
			goto d_edges;
		}
		to += vg;
		igraph_vector_push_back(&edges,(igraph_real_t)from);
		igraph_vector_push_back(&edges,(igraph_real_t)to);
	}
	igraph_eit_destroy(&eit);
	err = igraph_add_edges(&b,&edges,NULL);
	if(err) goto d_edges;

	/* maximum matching on b */
	igraph_vector_clear(&edges);
	err = bipartite_maximum_matching(&b,&edges);
	if(err) goto d_edges;

	/* Let M be the max matching, and N be E - M
	 * Define T as all unmatched vectices from U as well as all vertices
	 * reachable from those by going left-to-right along N and right-to-left along
	 * M.
	 * Define L = U - T, R = V \inter T
	 * C:= L + R
	 * C is a minimum vertex cover
	 */
	err = igraph_vector_init_seq(&n,0,igraph_ecount(&b)-1);
	if(err) goto d_edges;

	err = vector_diff(&n,&edges);
	if(err) goto d_n;

	err = igraph_vector_init(&c,vg);
	if(err) goto d_n;
	igraph_vector_clear(&c);

	/* matched vertices : S */
	err = igraph_vector_init(&s,vg);
	if(err) goto d_c;
	igraph_vector_clear(&s);

	for(i = 0; i < igraph_vector_size(&edges); i++)
	{
		err = igraph_edge(&b,VECTOR(edges)[i],&from,&to);
		if(err) goto d_s;

		igraph_vector_push_back(&s,from);
	}
	/* we may have inserted the same vertex multiple times */
	err = vector_uniq(&s);
	if(err) goto d_s;

	/* unmatched */
	err = igraph_vector_init_seq(&t,0,vg-1);
	if(err) goto d_s;

	err = vector_diff(&t,&s);
	if(err) goto d_t;

	/* alternating paths
	 */
	err = igraph_vector_copy(&todo,&t);
	if(err) goto d_t;

	err = igraph_vector_init(&next,vg);
	if(err) goto d_todo;
	igraph_vector_clear(&next);
	do {
		vector_uniq(&todo);
		added = 0;
		for(i = 0; i < igraph_vector_size(&todo); i++)
		{
			if(VECTOR(todo)[i] < vg)
			{
				/* scan edges */
				err = igraph_es_adj(&es,VECTOR(todo)[i],IGRAPH_OUT);
				if(err) goto d_next;
				err = igraph_eit_create(&b,es,&eit);
				if(err)
				{
					igraph_es_destroy(&es);
					goto d_next;
				}
				for(IGRAPH_EIT_RESET(eit); !IGRAPH_EIT_END(eit); IGRAPH_EIT_NEXT(eit))
				{
					if(igraph_vector_binsearch(&n,IGRAPH_EIT_GET(eit),NULL))
					{
						err = igraph_edge(&b,IGRAPH_EIT_GET(eit),&from,&to);
						if(err)
						{
							igraph_eit_destroy(&eit);
							igraph_es_destroy(&es);
							goto d_next;
						}
						if(!igraph_vector_binsearch(&t,to,NULL))
						{
							igraph_vector_push_back(&next,to);
							added = 1;
						}
					}
				}
			}
			else
			{
				/* scan edges */
				err = igraph_es_adj(&es,VECTOR(todo)[i],IGRAPH_IN);
				if(err) goto d_next;
				err = igraph_eit_create(&b,es,&eit);
				if(err)
				{
					igraph_es_destroy(&es);
					goto d_next;
				}
				for(IGRAPH_EIT_RESET(eit); !IGRAPH_EIT_END(eit); IGRAPH_EIT_NEXT(eit))
				{
					if(igraph_vector_binsearch(&edges,IGRAPH_EIT_GET(eit),NULL))
					{
						err = igraph_edge(&b,IGRAPH_EIT_GET(eit),&from,&to);
						if(err)
						{
							igraph_eit_destroy(&eit);
							igraph_es_destroy(&es);
							goto d_next;
						}
						if(!igraph_vector_binsearch(&t,to,NULL))
						{
							igraph_vector_push_back(&next,from);
							added = 1;
						}
					}
				}
			}
			igraph_es_destroy(&es);
			igraph_eit_destroy(&eit);
		}
		igraph_vector_append(&t,&todo);
		igraph_vector_clear(&todo);
		igraph_vector_append(&todo,&next);
		igraph_vector_clear(&next);
	} while(added);

	err = igraph_vector_init_seq(&l,0,vg-1);
	if(err) goto d_t;

	err = vector_diff(&l,&t);
	if(err) goto d_l;

	err = igraph_vector_update(&c,&l);
	if(err) goto d_l;

	err = igraph_vector_init(&r,vg);
	if(err) goto d_l;
	igraph_vector_clear(&r);

	/* compute V \inter T */
	for(i = 0; i < igraph_vector_size(&t); i++)
	{
		if(VECTOR(t)[i] >= vg)
			igraph_vector_push_back(&r,VECTOR(t)[i]);
	}

	igraph_vector_add_constant(&r,(igraph_real_t)-vg);
	err = vector_union(&c,&r);
	if(err) goto d_r;

	/* our antichain is U - C */
	res = malloc(sizeof(igraph_vector_t));
	if(res == NULL) goto d_r;

	err = igraph_vector_init_seq(res,0,vg-1);
	if(err) goto f_res;

	err = vector_diff(res,&c);
	if(err) goto d_res;

	goto ret;
d_res:
	igraph_vector_destroy(res);
f_res:
	free(res);
	res = NULL;
ret:
d_r:
	igraph_vector_destroy(&r);
d_l:
	igraph_vector_destroy(&l);
d_next:
	igraph_vector_destroy(&next);
d_todo:
	igraph_vector_destroy(&todo);
d_t:
	igraph_vector_destroy(&t);
d_s:
	igraph_vector_destroy(&s);
d_c:
	igraph_vector_destroy(&c);
d_n:
	igraph_vector_destroy(&n);
d_edges:
	igraph_vector_destroy(&edges);
d_b:
	igraph_destroy(&b);
error:
	igraph_destroy(&gstar);
	return res;
}

/* Lowest Single Ancestor:
 * The single lowest ancestor of a node is the vertex of maximum depth on all
 * paths between the root of the dag (single source) and this node.
 *
 * The simplest way to compute this is to build a specialized tree and do LCA
 * (lowest common ancestor of two nodes) queries on it.
 * See "New Common Ancestor Problems in Trees and Directed Acyclic Graphs" by
 * Johannes Fischer and Daniel H. Huson (2008).
 * While this paper recommends the use of a LCA algorithm that supports online
 * updates to the tree, it is quite difficult to implement correctly. The LCA
 * algorithm we use is with O(n) prepocessing time and O(1) queries, making the
 * LSA algorithm O(n^2).
 */

igraph_vector_t *ggen_analyze_lowest_single_ancestor(igraph_t *g)
{
	unsigned long i,v,l,vid,r;
	int err = 0;
	igraph_vector_t toposort,itopo;
	igraph_vector_t *lsa;
	igraph_t tree;
	igraph_vs_t vs;
	igraph_vit_t vit;
	lca_metadata md;

	if(g == NULL)
		return NULL;

	err = igraph_vector_init(&toposort,igraph_vcount(g));
	if(err) return NULL;

	err = igraph_topological_sorting(g,&toposort,IGRAPH_OUT);
	if(err) goto d_tp;

	/* build a reverse index of the toposort */
	err = igraph_vector_init(&itopo,igraph_vcount(g));
	if(err) goto d_tp;

	for(i = 0; i < igraph_vcount(g); i++)
	{
		v = VECTOR(toposort)[i];
		VECTOR(itopo)[v] = i;
	}

	err = igraph_empty(&tree,1,IGRAPH_DIRECTED);
	if(err) goto d_i;

	lsa = calloc(1,sizeof(igraph_vector_t*));
	if(lsa == NULL) goto cleanup;

	err = igraph_vector_init(lsa,igraph_vcount(g));
	if(err) goto f_l;

	for(v = 1; v < igraph_vcount(g); v++)
	{
		vid = VECTOR(toposort)[v];

		tree_lca_metadata_init(&tree,&md);
		tree_lca_preprocessing(&tree,0,&md);

		/* iterate over parents of v in g
		 * The lsa of a node is the LCA of all its parents in our
		 * special tree.
		 */
		igraph_vs_adj(&vs, vid, IGRAPH_IN);
		igraph_vit_create(g,vs,&vit);
		l = VECTOR(itopo)[IGRAPH_VIT_GET(vit)];
		IGRAPH_VIT_NEXT(vit);

		for(vit;!IGRAPH_VIT_END(vit); IGRAPH_VIT_NEXT(vit))
		{
			tree_lca_query(&tree,l,VECTOR(itopo)[IGRAPH_VIT_GET(vit)],&r,&md);
			l = r;
		}

		igraph_vit_destroy(&vit);
		igraph_vs_destroy(&vs);
		tree_lca_metadata_free(&tree,&md);

		// update tree
		err = igraph_add_vertices(&tree,1,NULL);
		if(err) goto d_l;

		err = igraph_add_edge(&tree,l,v);
		if(err) goto d_l;

		VECTOR(*lsa)[vid] = VECTOR(toposort)[l];
	}
	goto cleanup;
d_l:
	igraph_vector_destroy(lsa);
f_l:
	free(lsa);
	lsa = NULL;
cleanup:
	igraph_destroy(&tree);
d_i:
	igraph_vector_destroy(&itopo);
d_tp:
	igraph_vector_destroy(&toposort);
	return lsa;
}

