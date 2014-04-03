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
/* An implementation of offline lca query, with a prepocessing in O(n) and
 * query in O(1).
 * - we need a fast LCA tree data structure.
 * - and the LCA algorithm itself.
 * See "On finding lowest common ancestors: simplification and parallelization"
 * by Baruch Schieber and Uzi Vishkin (1988).
 */

#include <igraph/igraph.h>
#include "tree-lowest-common-ancestor.h"
#include "error.h"

/* a few utility functions for repetitive actions in the other functions */

/* find the index of a key in the table. Could be implemented faster with a
 * real hash table. */
static inline int head_search(head_t *h, unsigned long key, unsigned long *index)
{
	unsigned long i;
	for(i = 0; i < h->size; i++)
		if(h->table[i].key == key)
		{
			*index = i;
			return GGEN_SUCCESS;
		}
	return GGEN_FAILURE;
}

/* Find the father of a vid in the tree. Aborts program if we call it with a
 * node with no father. */
static inline int tree_father(const igraph_t *tree, unsigned long vid, unsigned long *ret)
{
	int err;
	unsigned long father;
	igraph_vs_t vs;
	igraph_vit_t vit;

	ggen_error_start_stack();
	GGEN_CHECK_IGRAPH(igraph_vs_adj(&vs, vid, IGRAPH_IN));
	GGEN_FINALLY(igraph_vs_destroy,&vs);
	GGEN_CHECK_IGRAPH(igraph_vit_create(tree,vs,&vit));
	GGEN_FINALLY(igraph_vit_destroy,&vit);
	father = (unsigned long)IGRAPH_VIT_GET(vit);
	*ret = father;
	ggen_error_clean(1);
	return GGEN_SUCCESS;
ggen_error_label:
	*ret = 0;
	return GGEN_FAILURE;
}

int tree_lca_metadata_init(igraph_t *tree, lca_metadata *m)
{
	unsigned long i;
	ggen_error_start_stack();
	m->tsize = igraph_vcount(tree);
	m->table = calloc(m->tsize,sizeof(unsigned long *));
	GGEN_CHECK_ALLOC(m->table);
	GGEN_FINALLY3(free,m->table,1);
	m->table[0] = calloc(m->tsize*3,sizeof(unsigned long));
	GGEN_CHECK_ALLOC(m->table[0]);
	GGEN_FINALLY3(free,m->table[0],1);

	for(i = 0; i < m->tsize; i++)
	{
		m->table[i] = m->table[0] + i*3;
	}
	m->head.table = calloc(m->tsize,sizeof(head_e));
	GGEN_CHECK_ALLOC(m->head.table);
	m->head.size = 0;
	ggen_error_clean(1);
	return GGEN_SUCCESS;
ggen_error_label:
	return GGEN_FAILURE;
}

void tree_lca_metadata_free(lca_metadata *m)
{
	free(m->table[0]);
	free(m->table);
	free(m->head.table);
}

/* meta->table access macros */
#define INLABEL(t,v) (t)[v][0]
#define ASCENDANT(t,v) (t)[v][1]
#define LEVEL(t,v) (t)[v][2]

/* a bfs callback. Builds the meta->table during the bfs. */
static igraph_bool_t  tree_ascendant_bfs_callback(const igraph_t *tree,
	igraph_integer_t vid, igraph_integer_t pred, igraph_integer_t succ,
	igraph_integer_t rank, igraph_integer_t dist, void *extra)
{
	unsigned long **table = (unsigned long **)extra;
	unsigned long father;

	ggen_error_start_stack();
	GGEN_CHECK_INTERNAL_ERRNO(tree_father(tree,vid,&father));
	// compute ascendant
	if(INLABEL(table,vid) == INLABEL(table,father))
	{
		ASCENDANT(table,vid) = ASCENDANT(table,father);
	}
	else
	{
		unsigned long i = log2(INLABEL(table,vid) - (INLABEL(table,vid) & (INLABEL(table,vid)-1)));
		ASCENDANT(table,vid) = ASCENDANT(table,father) + (1 << i);
	}
	ggen_error_clean(1);
	return GGEN_SUCCESS;
ggen_error_label:
	return GGEN_FAILURE;
}

/* builds recursively a preorder numbering of all vertices in the tree.
 * Also fills a vector containing the size of the subtree of a root (includes
 * the current node).
 */
static int tree_build_preorder(igraph_t *tree, unsigned long node,
	igraph_vector_t *preorder, igraph_vector_t *size, unsigned long *value)
{
	unsigned long sz = 0;
	igraph_vs_t vs;
	igraph_vit_t vit;

	ggen_error_start_stack();
	// save current value
	sz = *value;
	// asign preorder to node
	*value = *value +1;
	VECTOR(*preorder)[node] = *value;

	// find children and recurse
	GGEN_CHECK_IGRAPH(igraph_vs_adj(&vs, node, IGRAPH_OUT));
	GGEN_FINALLY(igraph_vs_destroy,&vs);
	GGEN_CHECK_IGRAPH(igraph_vit_create(tree,vs,&vit));
	GGEN_FINALLY(igraph_vit_destroy,&vit);
	for(vit;!IGRAPH_VIT_END(vit); IGRAPH_VIT_NEXT(vit))
	{
		GGEN_CHECK_INTERNAL_ERRNO(tree_build_preorder(tree,IGRAPH_VIT_GET(vit),preorder,size,value));
	}
	// compute size of subtree
	sz = *value - sz;
	VECTOR(*size)[node] = sz;
	ggen_error_clean(1);
	return GGEN_SUCCESS;
ggen_error_label:
	return GGEN_FAILURE;
}

/* Preprocess a tree to answer LCA queries in constant time.
 * See paper for full explanation of the various steps.
 */
int tree_lca_preprocessing(igraph_t *tree, unsigned long root, lca_metadata *m)
{
	/* INLABEL(v) */
	/* first step, compute the preorder of each vertex in the tree. */
	igraph_vector_t preorder, size, dist;
	unsigned long v = 0, l, i, father;

	ggen_error_start_stack();
	GGEN_CHECK_IGRAPH(igraph_vector_init(&preorder,igraph_vcount(tree)));
	GGEN_FINALLY(igraph_vector_destroy,&preorder);
	GGEN_CHECK_IGRAPH(igraph_vector_init(&size,igraph_vcount(tree)));
	GGEN_FINALLY(igraph_vector_destroy,&size);
	GGEN_CHECK_INTERNAL_ERRNO(tree_build_preorder(tree,root,&preorder,&size,&v));

	/* second step, compute inlabel(v) */
	for(v = 0; v < igraph_vcount(tree); v++)
	{
		// step 2.1
		i = (unsigned long)(VECTOR(preorder)[v] -1);
		i = i ^ (unsigned long)(VECTOR(preorder)[v]+VECTOR(size)[v] -1);
		i = (unsigned long)log2(i);
		// step 2.2
		l = ((unsigned long)(VECTOR(preorder)[v] + VECTOR(size)[v] -1)) >> i;
		INLABEL(m->table,v) = l << i;
	}
	/* ASCENDANT(v) & LEVEL(v)*/
	GGEN_CHECK_IGRAPH(igraph_vector_init(&dist,igraph_vcount(tree)));
	GGEN_FINALLY(igraph_vector_destroy,&dist);

	l = ceil(log2(igraph_vcount(tree)+1)) -1;
	ASCENDANT(m->table,0) = 1 << l;

	GGEN_CHECK_INTERNAL_DO(igraph_bfs(tree,root, NULL, IGRAPH_OUT, 0, NULL, NULL, NULL, NULL, NULL, NULL, &dist, tree_ascendant_bfs_callback,m->table));
	/* Finish LEVEL(v) */
	for(v = 0; v < igraph_vcount(tree); v++)
	{
		LEVEL(m->table,v) = VECTOR(dist)[v];
	}
	/* HEAD table */
	for(v = 0; v < igraph_vcount(tree); v++)
	{
		if(head_search(&(m->head), INLABEL(m->table,v),&i) == GGEN_FAILURE);
		{
			m->head.table[m->head.size].key = INLABEL(m->table,v);
			m->head.table[m->head.size].value = v;
			m->head.size++;
		}
	}
	for(v = 1; v < igraph_vcount(tree); v++)
	{
		GGEN_CHECK_INTERNAL_ERRNO(tree_father(tree,v,&father));
		if(INLABEL(m->table,v) != INLABEL(m->table,father))
		{
			GGEN_CHECK_INTERNAL_ERRNO(head_search(&(m->head), INLABEL(m->table,v),&i));
			m->head.table[i].value = v;
		}
	}
	ggen_error_clean(1);
	return GGEN_SUCCESS;
ggen_error_label:
	return GGEN_FAILURE;
}

int tree_lca_query(igraph_t *tree, unsigned long u, unsigned long v,
	unsigned long *ret, lca_metadata *m)
{
	unsigned long i,b,c,ci,j,iz,ubar,vbar,k,iw,w;
	unsigned long tmp;
	unsigned long **table = m->table;
	head_t *head = &(m->head);

	ggen_error_start_stack();
	if(INLABEL(table,u) == INLABEL(table,v))
	{
		if(LEVEL(table,u) <= LEVEL(table,v))
			*ret = u;
		else
			*ret = v;
	}
	else
	{
		// step 1
		i = (unsigned long) log2(INLABEL(table,u) ^ INLABEL(table,v));
		b = ((INLABEL(table,u) >> (i+1)) << (i+1)) + (1 << i);
		// step 2
		c = ASCENDANT(table,u) & ASCENDANT(table,v);
		ci = (c >> i) << i;
		j = log2(ci - (ci & (ci -1)));
		iz = ((INLABEL(table,u) >> (j+1)) << (j+1)) + (1 << j);
		// step 3
		if(INLABEL(table,u) == iz)
			ubar = u;
		else
		{
			k = ((1 << j) -1) & ASCENDANT(table,u);
			if(k != 0)
				k = log2(k);
			iw = ((INLABEL(table,u) >> (k+1)) << (k+1)) + (1 << k);
			GGEN_CHECK_INTERNAL(head_search(head,iw,&tmp));
			w = head->table[tmp].value;
			GGEN_CHECK_INTERNAL_ERRNO(tree_father(tree,w,&ubar));
		}
		if(INLABEL(table,v) == iz)
			vbar = v;
		else
		{
			k = ((1 << j) -1) & ASCENDANT(table,v);
			if(k != 0)
				k = log2(k);
			iw = ((INLABEL(table,v) >> (k+1)) << (k+1)) + (1 << k);
			GGEN_CHECK_INTERNAL(head_search(head,iw,&tmp));
			w = head->table[tmp].value;
			GGEN_CHECK_INTERNAL_ERRNO(tree_father(tree,w,&vbar));
		}
		if(LEVEL(table,ubar) <= LEVEL(table,vbar))
			*ret = ubar;
		else
			*ret = vbar;
	}
	ggen_error_clean(1);
	return GGEN_SUCCESS;
ggen_error_label:
	*ret = 0;
	return GGEN_FAILURE;
}


