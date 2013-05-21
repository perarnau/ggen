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

/* a few utility functions for repetitive actions in the other functions */

/* find the index of a key in the table. Could be implemented faster with a
 * real hash table. */
static inline unsigned long head_search(head_t *h, unsigned long key)
{
	unsigned long i;
	
	for(i = 0; i < h->size; i++)
		if(h->table[i].key == key)
			return i;
	return -1;
}

/* Find the father of a vid in the tree. Aborts program if we call it with a
 * node with no father. */
static inline unsigned long tree_father(const igraph_t *tree, unsigned long vid)
{
	int err;
	unsigned long father;
	igraph_vs_t vs;
	igraph_vit_t vit;

	igraph_vs_adj(&vs, vid, IGRAPH_IN);
	igraph_vit_create(tree,vs,&vit);
	father = (unsigned long)IGRAPH_VIT_GET(vit);
	igraph_vit_destroy(&vit);
	igraph_vs_destroy(&vs);
	return father;
}

int tree_lca_metadata_init(igraph_t *tree, lca_metadata *m)
{
	unsigned long i;
	
	m->table = calloc(igraph_vcount(tree),sizeof(unsigned long));
	for(i = 0; i < igraph_vcount(tree); i++)
		m->table[i] = calloc(3,sizeof(unsigned long));
	m->head.table = calloc(igraph_vcount(tree),sizeof(head_e));
	m->head.size = 0;
	return 0;
}

int tree_lca_metadata_free(igraph_t *tree, lca_metadata *m)
{
	unsigned long i;
	for(i = 0; i < igraph_vcount(tree); i++)
		free(m->table[i]);
	free(m->table);
	free(m->head.table);
	return 0;
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

	father = tree_father(tree,vid);
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
	return 0;
}

/* builds recursively a preorder numbering of all vertices in the tree.
 * Also fills a vector containing the size of the subtree of a root (includes
 * the current node).
 */
static void tree_build_preorder(igraph_t *tree, unsigned long node,
	igraph_vector_t *preorder, igraph_vector_t *size, unsigned long *value)
{
	unsigned long sz = 0;
	igraph_vs_t vs;
	igraph_vit_t vit;
	// save current value
	sz = *value;
	// asign preorder to node
	*value = *value +1;
	VECTOR(*preorder)[node] = *value;

	// find children and recurse
	igraph_vs_adj(&vs, node, IGRAPH_OUT);
	igraph_vit_create(tree,vs,&vit);
	for(vit;!IGRAPH_VIT_END(vit); IGRAPH_VIT_NEXT(vit))
	{
		tree_build_preorder(tree,IGRAPH_VIT_GET(vit),preorder,size,value);
	}
	igraph_vit_destroy(&vit);
	igraph_vs_destroy(&vs);
	// compute size of subtree
	sz = *value - sz;
	VECTOR(*size)[node] = sz;
}

/* Preprocess a tree to answer LCA queries in constant time.
 * See paper for full explanation of the various steps.
 */
int tree_lca_preprocessing(igraph_t *tree, unsigned long root, lca_metadata *m)
{
	/* INLABEL(v) */
	/* first step, compute the preorder of each vertex in the tree. */
	igraph_vector_t preorder,size;
	unsigned long v = 0;
	igraph_vector_init(&preorder,igraph_vcount(tree));
	igraph_vector_init(&size,igraph_vcount(tree));
	tree_build_preorder(tree,root,&preorder,&size,&v);

	/* second step, compute inlabel(v) */
	for(v = 0; v < igraph_vcount(tree); v++)
	{
		// step 2.1
		unsigned long i = (unsigned long)(VECTOR(preorder)[v] -1);
		i = i ^ (unsigned long)(VECTOR(preorder)[v]+VECTOR(size)[v] -1);
		i = (unsigned long)log2(i);
		// step 2.2
		unsigned long l = ((unsigned long)(VECTOR(preorder)[v] + VECTOR(size)[v] -1)) >> i;
		INLABEL(m->table,v) = l << i;
	}
	/* ASCENDANT(v) & LEVEL(v)*/
	igraph_vector_t dist;
	igraph_vector_init(&dist,igraph_vcount(tree));
	unsigned long l = ceil(log2(igraph_vcount(tree)+1)) -1;
	ASCENDANT(m->table,0) = 1 << l;
	igraph_bfs(tree,root,NULL,IGRAPH_OUT,0,NULL,NULL,NULL,NULL,NULL,NULL,
			&dist, tree_ascendant_bfs_callback,m->table);
	/* Finish LEVEL(v) */
	for(v = 0; v < igraph_vcount(tree); v++)
	{
		LEVEL(m->table,v) = VECTOR(dist)[v];
	}
	/* HEAD table */
	for(v = 0; v < igraph_vcount(tree); v++)
	{
		unsigned long i = head_search(&(m->head), INLABEL(m->table,v));
		if(i == (unsigned long)-1)
		{
			m->head.table[m->head.size].key = INLABEL(m->table,v);
			m->head.table[m->head.size].value = v;
			m->head.size++;
		}
	}
	for(v = 1; v < igraph_vcount(tree); v++)
	{
		unsigned long father;
		father = tree_father(tree,v);

		if(INLABEL(m->table,v) != INLABEL(m->table,father))
		{
			unsigned long i = head_search(&(m->head),INLABEL(m->table,v));
			m->head.table[i].value = v;
		}
	}
	return 0;
}

int tree_lca_query(igraph_t *tree, unsigned long u, unsigned long v,
	unsigned long *ret, lca_metadata *m)
{
	unsigned long **table = m->table;
	head_t *head = &(m->head);
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
		unsigned long i = (unsigned long) log2(INLABEL(table,u) ^ INLABEL(table,v));
		unsigned long b = ((INLABEL(table,u) >> (i+1)) << (i+1)) + (1 << i);
		// step 2
		unsigned long c = ASCENDANT(table,u) & ASCENDANT(table,v);
		unsigned long ci = (c >> i) << i;
		unsigned long j = log2(ci - (ci & (ci -1)));
		unsigned long iz = ((INLABEL(table,u) >> (j+1)) << (j+1)) + (1 << j);
		// step 3
		unsigned long ubar, vbar;
		if(INLABEL(table,u) == iz)
			ubar = u;
		else
		{
			unsigned long k = ((1 << j) -1) & ASCENDANT(table,u);
			unsigned long iw;
			if(k != 0)
				k = log2(k);
			iw = ((INLABEL(table,u) >> (k+1)) << (k+1)) + (1 << k);
			unsigned long w = head->table[head_search(head,iw)].value;
			ubar = tree_father(tree,w);

		}
		if(INLABEL(table,v) == iz)
			vbar = v;
		else
		{
			unsigned long k = ((1 << j) -1) & ASCENDANT(table,v);
			unsigned long iw;
			if(k != 0)
				k = log2(k);
			iw = ((INLABEL(table,v) >> (k+1)) << (k+1)) + (1 << k);
			unsigned long w = head->table[head_search(head,iw)].value;
			vbar = tree_father(tree,w);
		}
		if(LEVEL(table,ubar) <= LEVEL(table,vbar))
			*ret = ubar;
		else
			*ret = vbar;
	}
	return 0;
}


