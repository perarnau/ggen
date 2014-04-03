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
#include "error.h"

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

	ggen_error_start_stack();
	if(g == NULL)
		GGEN_SET_ERRNO(GGEN_EINVAL);

	v = igraph_vcount(g);
	GGEN_CHECK_IGRAPH(igraph_vector_init(&topology,v));
	GGEN_FINALLY(igraph_vector_destroy,&topology);

	GGEN_CHECK_IGRAPH(igraph_vector_init(&lengths,v));
	GGEN_FINALLY(igraph_vector_destroy,&lengths);

	GGEN_CHECK_IGRAPH(igraph_vector_init(&preds,v));
	GGEN_FINALLY(igraph_vector_destroy,&preds);

	res = malloc(sizeof(igraph_vector_t));
	GGEN_CHECK_ALLOC(res);
	GGEN_FINALLY3(free,res,1);

	GGEN_CHECK_IGRAPH(igraph_vector_init(res,v));
	GGEN_FINALLY3(igraph_destroy,res,1);

	// sort topologically the vertices
	GGEN_CHECK_IGRAPH(igraph_topological_sorting(g,&topology,IGRAPH_OUT));

	// find the best path incomming from every node
	igraph_vector_null(&lengths);
	igraph_vector_fill(&preds,-1);
	maxv = -1;
	for(i = 0; i < v; i++)
	{
		f = VECTOR(topology)[i];
		GGEN_CHECK_IGRAPH(igraph_vs_adj(&vs,f,IGRAPH_OUT));
		GGEN_FINALLY(igraph_vs_destroy,&vs);

		GGEN_CHECK_IGRAPH(igraph_vit_create(g,vs,&vit));
		GGEN_FINALLY(igraph_vit_destroy,&vit);

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
		// if everything went ok, we need to pop the destructor for vs
		// and vit
		ggen_error_pop(2);
	}
	// build the path, using preds and maxv
	f = 0;
	while(maxv != -1)
	{
		VECTOR(*res)[f++] = maxv;
		maxv = VECTOR(preds)[maxv];
	}

	// finish the path correctly, resizing and reversing the array
	GGEN_CHECK_IGRAPH(igraph_vector_resize(res,f));
	GGEN_CHECK_IGRAPH(igraph_vector_reverse(res));

	ggen_error_clean(1);
	return res;
ggen_error_label:
	return NULL;
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
	 *  - convert maximum matching to min vectex cover
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

	ggen_error_start_stack();
	if(g == NULL)
		GGEN_SET_ERRNO(GGEN_EINVAL);

	/* before creating the bipartite graph, we need all relations
	 * between any two vertices : the transitive closure of g */
	GGEN_CHECK_IGRAPH(igraph_copy(&gstar,g));
	GGEN_FINALLY(igraph_destroy,&gstar);

	GGEN_CHECK_IGRAPH(ggen_transform_transitive_closure(&gstar));


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
	GGEN_CHECK_IGRAPH(igraph_empty(&b,vg*2,1));
	GGEN_FINALLY(igraph_destroy,&b);

	/* id and id+vg will be a vertex in U and its copy in V,
	 * iterate over gstar edges to create edges in b
	 */
	GGEN_CHECK_IGRAPH(igraph_vector_init(&edges,igraph_ecount(&gstar)));
	GGEN_FINALLY(igraph_vector_destroy,&edges);
	igraph_vector_clear(&edges);

	GGEN_CHECK_IGRAPH(igraph_eit_create(&gstar,igraph_ess_all(IGRAPH_EDGEORDER_ID),&eit));
	GGEN_FINALLY(igraph_eit_destroy,&eit);

	for(IGRAPH_EIT_RESET(eit); !IGRAPH_EIT_END(eit); IGRAPH_EIT_NEXT(eit))
	{
		igraph_edge(&gstar,IGRAPH_EIT_GET(eit),&from,&to);
		to += vg;
		igraph_vector_push_back(&edges,(igraph_real_t)from);
		igraph_vector_push_back(&edges,(igraph_real_t)to);
	}
	// destroy eit
	ggen_error_pop(1);
	GGEN_CHECK_IGRAPH(igraph_add_edges(&b,&edges,NULL));

	/* maximum matching on b */
	igraph_vector_clear(&edges);
	GGEN_CHECK_INTERNAL(bipartite_maximum_matching(&b,&edges));

	/* Let M be the max matching, and N be E - M
	 * Define T as all unmatched vectices from U as well as all vertices
	 * reachable from those by going left-to-right along N and right-to-left along
	 * M.
	 * Define L = U - T, R = V \inter T
	 * C:= L + R
	 * C is a minimum vertex cover
	 */
	GGEN_CHECK_IGRAPH(igraph_vector_init_seq(&n,0,igraph_ecount(&b)-1));
	GGEN_FINALLY(igraph_vector_destroy,&n);

	GGEN_CHECK_INTERNAL(vector_diff(&n,&edges));

	GGEN_CHECK_IGRAPH(igraph_vector_init(&c,vg));
	GGEN_FINALLY(igraph_vector_destroy,&c);
	igraph_vector_clear(&c);

	/* matched vertices : S */
	GGEN_CHECK_IGRAPH(igraph_vector_init(&s,vg));
	GGEN_FINALLY(igraph_vector_destroy,&s);
	igraph_vector_clear(&s);

	for(i = 0; i < igraph_vector_size(&edges); i++)
	{
		igraph_edge(&b,VECTOR(edges)[i],&from,&to);
		igraph_vector_push_back(&s,from);
	}
	/* we may have inserted the same vertex multiple times */
	GGEN_CHECK_INTERNAL(vector_uniq(&s));

	/* unmatched */
	GGEN_CHECK_IGRAPH(igraph_vector_init_seq(&t,0,vg-1));
	GGEN_FINALLY(igraph_vector_destroy,&t);

	GGEN_CHECK_INTERNAL(vector_diff(&t,&s));

	/* alternating paths
	 */
	GGEN_CHECK_IGRAPH(igraph_vector_copy(&todo,&t));
	GGEN_FINALLY(igraph_vector_destroy,&todo);

	GGEN_CHECK_IGRAPH(igraph_vector_init(&next,vg));
	GGEN_FINALLY(igraph_vector_destroy,&next);
	igraph_vector_clear(&next);

	do {
		vector_uniq(&todo);
		added = 0;
		for(i = 0; i < igraph_vector_size(&todo); i++)
		{
			if(VECTOR(todo)[i] < vg)
			{
				/* scan edges */
				GGEN_CHECK_IGRAPH(igraph_es_adj(&es,VECTOR(todo)[i],IGRAPH_OUT));
				GGEN_FINALLY(igraph_es_destroy,&es);

				GGEN_CHECK_IGRAPH(igraph_eit_create(&b,es,&eit));
				GGEN_FINALLY(igraph_eit_destroy,&eit);

				for(IGRAPH_EIT_RESET(eit); !IGRAPH_EIT_END(eit); IGRAPH_EIT_NEXT(eit))
				{
					if(igraph_vector_binsearch(&n,IGRAPH_EIT_GET(eit),NULL))
					{
						igraph_edge(&b,IGRAPH_EIT_GET(eit),&from,&to);
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
				GGEN_CHECK_IGRAPH(igraph_es_adj(&es,VECTOR(todo)[i],IGRAPH_IN));
				GGEN_FINALLY(igraph_es_destroy,&es);

				GGEN_CHECK_IGRAPH(igraph_eit_create(&b,es,&eit));
				GGEN_FINALLY(igraph_eit_destroy,&eit);

				for(IGRAPH_EIT_RESET(eit); !IGRAPH_EIT_END(eit); IGRAPH_EIT_NEXT(eit))
				{
					if(igraph_vector_binsearch(&edges,IGRAPH_EIT_GET(eit),NULL))
					{
						igraph_edge(&b,IGRAPH_EIT_GET(eit),&from,&to);
						if(!igraph_vector_binsearch(&t,to,NULL))
						{
							igraph_vector_push_back(&next,from);
							added = 1;
						}
					}
				}
			}
			// destroy es and eit
			ggen_error_pop(2);
		}
		igraph_vector_append(&t,&todo);
		igraph_vector_clear(&todo);
		igraph_vector_append(&todo,&next);
		igraph_vector_clear(&next);
	} while(added);

	GGEN_CHECK_IGRAPH(igraph_vector_init_seq(&l,0,vg-1));
	GGEN_FINALLY(igraph_vector_destroy,&l);

	GGEN_CHECK_INTERNAL(vector_diff(&l,&t));

	GGEN_CHECK_IGRAPH(igraph_vector_update(&c,&l));

	GGEN_CHECK_IGRAPH(igraph_vector_init(&r,vg));
	GGEN_FINALLY(igraph_vector_destroy,&r);
	igraph_vector_clear(&r);

	/* compute V \inter T */
	for(i = 0; i < igraph_vector_size(&t); i++)
	{
		if(VECTOR(t)[i] >= vg)
			igraph_vector_push_back(&r,VECTOR(t)[i]);
	}

	igraph_vector_add_constant(&r,(igraph_real_t)-vg);
	GGEN_CHECK_INTERNAL(vector_union(&c,&r));

	/* our antichain is U - C */
	res = malloc(sizeof(igraph_vector_t));
	GGEN_CHECK_ALLOC(res);
	GGEN_FINALLY3(free,res,1);

	GGEN_CHECK_IGRAPH(igraph_vector_init_seq(res,0,vg-1));
	GGEN_FINALLY3(igraph_vector_destroy,res,1);

	GGEN_CHECK_INTERNAL(vector_diff(res,&c));

	ggen_error_clean(1);
	return res;
ggen_error_label:
	return NULL;
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
	unsigned long i,v,l,m,vid,r;
	int err = 0;
	igraph_vector_t toposort,itopo;
	igraph_vector_t *lsa;
	igraph_t tree;
	igraph_vs_t vs;
	igraph_vit_t vit;
	lca_metadata md;

	ggen_error_start_stack();
	if(g == NULL)
		GGEN_SET_ERRNO(GGEN_EINVAL);

	GGEN_CHECK_IGRAPH(igraph_vector_init(&toposort,igraph_vcount(g)));
	GGEN_FINALLY(igraph_vector_destroy,&toposort);

	GGEN_CHECK_IGRAPH(igraph_topological_sorting(g,&toposort,IGRAPH_OUT));

	/* build a reverse index of the toposort */
	GGEN_CHECK_IGRAPH(igraph_vector_init(&itopo,igraph_vcount(g)));
	GGEN_FINALLY(igraph_vector_destroy,&itopo);

	for(i = 0; i < igraph_vcount(g); i++)
	{
		v = VECTOR(toposort)[i];
		VECTOR(itopo)[v] = i;
	}

	GGEN_CHECK_IGRAPH(igraph_empty(&tree,1,IGRAPH_DIRECTED));
	GGEN_FINALLY(igraph_destroy,&tree);

	lsa = calloc(1,sizeof(igraph_vector_t*));
	GGEN_CHECK_ALLOC(lsa);
	GGEN_FINALLY3(free,lsa,1);

	GGEN_CHECK_IGRAPH(igraph_vector_init(lsa,igraph_vcount(g)));
	GGEN_FINALLY3(igraph_vector_destroy,lsa,1);

	/* lsa of single source is single source */
	v = VECTOR(toposort)[0];
	VECTOR(*lsa)[v] = v;
	for(v = 1; v < igraph_vcount(g); v++)
	{
		vid = VECTOR(toposort)[v];

		GGEN_CHECK_INTERNAL_ERRNO(tree_lca_metadata_init(&tree,&md));
		GGEN_FINALLY(tree_lca_metadata_free,&md);
		GGEN_CHECK_INTERNAL_ERRNO(tree_lca_preprocessing(&tree,0,&md));

		/* iterate over parents of v in g
		 * The lsa of a node is the LCA of all its parents in our
		 * special tree.
		 */
		GGEN_CHECK_IGRAPH(igraph_vs_adj(&vs, vid, IGRAPH_IN));
		GGEN_FINALLY(igraph_vs_destroy,&vs);
		GGEN_CHECK_IGRAPH(igraph_vit_create(g,vs,&vit));
		GGEN_FINALLY(igraph_vit_destroy,&vit);

		l = VECTOR(itopo)[IGRAPH_VIT_GET(vit)];
		IGRAPH_VIT_NEXT(vit);

		for(vit;!IGRAPH_VIT_END(vit); IGRAPH_VIT_NEXT(vit))
		{
			m = VECTOR(itopo)[IGRAPH_VIT_GET(vit)];
			GGEN_CHECK_INTERNAL_ERRNO(tree_lca_query(&tree,l,m,&r,&md));
			l = r;
		}

		// destroy metadata, vs and vit
		ggen_error_pop(3);

		// update tree
		GGEN_CHECK_IGRAPH(igraph_add_vertices(&tree,1,NULL));

		// v is the id of vid in tree
		GGEN_CHECK_IGRAPH(igraph_add_edge(&tree,l,v));

		VECTOR(*lsa)[vid] = VECTOR(toposort)[l];
	}
	ggen_error_clean(1);
	return lsa;
ggen_error_label:
	return NULL;
}

/* An approximation algorithm to list edge-disjoint paths in the graph.
 * Note that:
 * - this is a NP-Complete problem, so this algorithm is an approximation
 * - we make no guaranties on the quality of this algorithm.
 * Returns a vector, listing for each edge id the index of its path.
 *
 * The algorithm simply find a path in the graph, and removes it until no edges
 * are left. We operate on a deep copy, the input graph is not destroyed by the
 * analysis.
 */
igraph_vector_t * ggen_analyze_edge_disjoint_paths(igraph_t *g)
{
	igraph_vector_t *paths = NULL, *p;
	igraph_vector_t indegrees, outdegrees;
	igraph_es_t es;
	igraph_t copy;
	igraph_integer_t eid, from, to;
	igraph_vector_ptr_t edges;
	igraph_warning_handler_t *handler;
	unsigned long vcount, ecount, source, sink, nbpaths, i;
	int err = 0;

	ggen_error_start_stack();
	if(g == NULL)
		GGEN_SET_ERRNO(GGEN_EINVAL);

	GGEN_CHECK_IGRAPH(igraph_copy(&copy,g));
	GGEN_FINALLY(igraph_destroy,&copy);

	paths = calloc(1,sizeof(igraph_vector_t));
	GGEN_CHECK_ALLOC(paths);
	GGEN_FINALLY3(free,paths,1);

	vcount = igraph_vcount(&copy);
	GGEN_CHECK_IGRAPH(igraph_vector_init(&indegrees,vcount));
	GGEN_FINALLY(igraph_vector_destroy,&indegrees);

	GGEN_CHECK_IGRAPH(igraph_vector_init(&outdegrees,vcount));
	GGEN_FINALLY(igraph_vector_destroy,&outdegrees);

	ecount = igraph_ecount(&copy);

	GGEN_CHECK_IGRAPH(igraph_vector_init(paths,ecount));
	GGEN_FINALLY3(igraph_vector_destroy,paths,1);

	GGEN_CHECK_IGRAPH(igraph_vector_ptr_init(&edges, vcount));
	IGRAPH_VECTOR_PTR_SET_ITEM_DESTRUCTOR(&edges, igraph_vector_destroy);
	GGEN_FINALLY(igraph_vector_ptr_destroy_all,&edges);

	for(i = 0; i < vcount; i++)
	{
		VECTOR(edges)[i] = calloc(1,sizeof(igraph_vector_t));
		GGEN_CHECK_ALLOC(VECTOR(edges)[i]);
		GGEN_CHECK_IGRAPH_VECTPTR(igraph_vector_init(VECTOR(edges)[i],0),edges,i);
	}

	nbpaths = 0;
	while(ecount)
	{
		/* list vertex degrees */
		igraph_degree(&copy, &indegrees, igraph_vss_all(), IGRAPH_IN,0);
		igraph_degree(&copy, &outdegrees, igraph_vss_all(), IGRAPH_OUT,0);

		/* find a source in the graph */
		for(i = 0; i < vcount; i++)
		{
			if(VECTOR(indegrees)[i] == 0 && VECTOR(outdegrees)[i] != 0)
				break;
		}
		if(i == vcount)
			GGEN_SET_ERRNO(GGEN_EINVAL); /* error ! */
		source = i;

		/* build the shortest paths to all vertices */
		/* igrore warnings, we know some of the vertices don't have
		 * paths from source
		 */
		handler = igraph_set_warning_handler(igraph_warning_handler_ignore);
		igraph_get_shortest_paths(&copy, NULL, &edges, source,
					igraph_vss_all(), IGRAPH_OUT);
		igraph_set_warning_handler(handler);

		/* find a sink that has a path from source */
		for(i = 0; i < vcount; i++)
		{
			if(VECTOR(outdegrees)[i] == 0 && VECTOR(indegrees)[i] != 0)
			{
				/* check that this sink is connected to source
				 */
				if(igraph_vector_size(VECTOR(edges)[i]) != 0)
					break;
			}
		}
		if(i == vcount)
			GGEN_SET_ERRNO(GGEN_EINVAL); /* error */
		sink = i;

		/* remove the edges we just found from the graph, but before,
		 * save the path in the result vector
		 */
		p = VECTOR(edges)[sink];
		for(i = 0; i < igraph_vector_size(p); i++)
		{
			igraph_edge(&copy,VECTOR(*p)[i],&from,&to);
			GGEN_CHECK_IGRAPH(igraph_get_eid(g, &eid, from, to, 1, 0));
			VECTOR(*paths)[eid] = nbpaths;
		}
		GGEN_CHECK_IGRAPH(igraph_es_vector(&es,p));
		GGEN_CHECK_IGRAPH(igraph_delete_edges(&copy, es));
		ecount = igraph_ecount(&copy);
		nbpaths++;
	}
	ggen_error_clean(1);
	return paths;
ggen_error_label:
	return NULL;
}

