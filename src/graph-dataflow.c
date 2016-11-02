/* Copyright Swann Perarnau 2009
*
*   contact : swann.perarnau@imag.fr
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
#include "error.h"

/**********************************************************
 * Utils
 *********************************************************/

/* A lot of our algorithms rely on adding a single task and figuring out
 * the dependencies it has because it's reading data that might have been wrote
 * before.
 * To manage that, we use a long vector remembering the last task that wrote into
 * a region of memory. If that vector contains a valid task id, then we add an
 * edge from that last task to the new one.
 * TODO: error checks
 */

static inline unsigned long addtask(igraph_t *g)
{
	unsigned long curtask = igraph_vcount(g);
	igraph_add_vertices(g, 1, NULL);
	return curtask;
}

static inline void raw_edge_1d(igraph_vector_long_t *lastwrite,
			       unsigned long i, igraph_t *g,
			       unsigned long task)
{
	long todo = VECTOR(*lastwrite)[i];
	unsigned long from;
	igraph_integer_t eid;
	if(todo != -1)
	{
		from = VECTOR(*lastwrite)[i];
		igraph_add_edge(g, from, task);
		igraph_get_eid(g, &eid, from, task, 1,0);
		SETEAN(g, "x", eid, i);
	}

}

static inline void raw_edge_2d(igraph_matrix_long_t *lastwrite,
			       unsigned long i, unsigned long j,
			       igraph_t *g, unsigned long task)
{
	long todo = MATRIX(*lastwrite,i,j);
	unsigned long from;
	igraph_integer_t eid;
	if(todo != -1)
	{
		from = MATRIX(*lastwrite,i,j);
		igraph_add_edge(g, from, task);
		igraph_get_eid(g, &eid, from, task, 1,0);
		SETEAN(g, "x", eid, i);
		SETEAN(g, "y", eid, j);
	}
}

/**********************************************************
 * Methods
 *********************************************************/

/* Block Cholesky factorization, lifted for KaStORS/Plasma code.
 * Matrix is of size n*n blocks.
 */
igraph_t *ggen_generate_cholesky(unsigned long size)
{
	igraph_t *g = NULL;
	igraph_matrix_long_t lastwrite;
	unsigned long k, m, n, task;

	ggen_error_start_stack();

	g = malloc(sizeof(igraph_t));
	GGEN_CHECK_ALLOC(g);
	GGEN_FINALLY3(free,g,1);

	GGEN_CHECK_IGRAPH(igraph_empty(g,0,1));
	GGEN_FINALLY3(igraph_destroy,g,1);

	GGEN_CHECK_IGRAPH(igraph_matrix_long_init(&lastwrite, size, size));
	GGEN_FINALLY(igraph_matrix_destroy, &lastwrite);
	igraph_matrix_long_fill(&lastwrite, -1);

	/* There's a single matrix using inout, use lastwrite to create the right
	 * edges.
	 */
	for(k = 0; k < size; k++)
	{
		/* potrf inout [k,k]*/
		task = addtask(g);
		SETVAS(g, "kernel", task, "potrf");
		raw_edge_2d(&lastwrite, k, k, g, task);
		MATRIX(lastwrite, k, k) = task;

		for(m = k+1; m < size; m++)
		{
			/* trsm: in [k,k] inout [k,m] */
			task = addtask(g);
			SETVAS(g, "kernel", task, "trsm");
			raw_edge_2d(&lastwrite, k, k, g, task);
			raw_edge_2d(&lastwrite, k, m, g, task);
			MATRIX(lastwrite, k, m) = task;
		}
		for(m = k+1; m < size; m++)
		{
			/* syrk: in [k,m] inout [m,m] */
			task = addtask(g);
			SETVAS(g, "kernel", task, "syrk");
			raw_edge_2d(&lastwrite, k, m, g, task);
			raw_edge_2d(&lastwrite, m, m, g, task);
			MATRIX(lastwrite, m, m) = task;

			for(n = k+1; n < m; n++)
			{
				/* gemm: in [k,n] in [k,m] inout [n,m] */
				task = addtask(g);
				SETVAS(g, "kernel", task, "gemm");
				raw_edge_2d(&lastwrite, k, n, g, task);
				raw_edge_2d(&lastwrite, k, m, g, task);
				raw_edge_2d(&lastwrite, n, m, g, task);
				MATRIX(lastwrite, n, m) = task;
			}
		}
	}
	ggen_error_clean(1);
	return g;
ggen_error_label:
	return NULL;

}

/* lifted from the KaStORS/BCS OpenMP task suite */
static int sparselu_genmat(igraph_matrix_bool_t *matrix, unsigned long size)
{
	unsigned long ii, jj, kk;
	igraph_bool_t empty;

	for(ii = 0; ii < size; ii++)
	{
		for(jj = 0; jj < size; jj++)
		{
			empty = 0;
			if((ii<jj) && (ii%3 !=0)) empty = 1;
			if((ii>jj) && (jj%3 !=0)) empty = 1;
			if(ii%2==1) empty = 1;
			if(jj%2==1) empty = 1;
			if(ii==jj) empty = 0;
			if(ii==jj-1) empty = 0;
			if(ii-1 == jj) empty = 0;
			MATRIX(*matrix, ii, jj) = !empty;
		}
	}
	return 0;
}

/* LU: generate the graph from a LU decomposition of a possibly sparse matrix of
 * size size*size BLOCKS.
 */
static int generate_lu(igraph_t *g, unsigned long size, igraph_matrix_bool_t nonempty)
{
	igraph_matrix_long_t lastwrite;
	unsigned long ii, jj, kk, to, from, task, lastlu;

	ggen_error_start_stack();

	GGEN_CHECK_IGRAPH(igraph_empty(g,0,1));
	GGEN_FINALLY3(igraph_destroy,g,1);
	GGEN_CHECK_IGRAPH(igraph_matrix_long_init(&lastwrite, size, size));
	GGEN_FINALLY(igraph_matrix_destroy, &lastwrite);
	igraph_matrix_long_fill(&lastwrite, -1);

	/* run through the motions of the algorithm, creating tasks
	 * and checking last writers.
	 */
	for(kk = 0; kk < size; kk++)
	{
		/* lu task: inout [kk*size +kk]*/
		lastlu = addtask(g);
		SETVAS(g, "kernel", lastlu, "lu");
		raw_edge_2d(&lastwrite, kk, kk, g, lastlu);
		MATRIX(lastwrite, kk, kk) = lastlu;

		for(jj = kk+1; jj < size; jj++)
			if(MATRIX(nonempty, kk, jj))
			{
				/* fwd: in [kk*size + kk] inout [kk*size+jj] */
				task = addtask(g);
				SETVAS(g, "kernel", task, "fwd");
				raw_edge_2d(&lastwrite, kk, kk, g, task);
				raw_edge_2d(&lastwrite, kk, jj, g, task);
				MATRIX(lastwrite, kk, jj) = task;
			}
		for(ii = kk+1; ii < size; ii++)
			if(MATRIX(nonempty, ii, kk))
			{
				/* bdiv: in [kk*size +kk] inout: [ii*size +kk]
				 */
				task = addtask(g);
				SETVAS(g, "kernel", task, "bdiv");
				raw_edge_2d(&lastwrite, kk, kk, g, task);
				raw_edge_2d(&lastwrite, ii, kk, g, task);
				MATRIX(lastwrite, ii, kk) = task;
			}
		for(ii = kk+1; ii < size; ii++)
			if(MATRIX(nonempty, ii, kk))
				for(jj = kk+1; jj < size; jj++)
					if(MATRIX(nonempty, kk, jj))
					{
						/* bmod: in [ii*size +kk]
						 *       in [kk*size +jj]
						 *       inout [ii*size +jj]
						 */
						MATRIX(nonempty, ii, jj) = 1;
						task = addtask(g);
						SETVAS(g, "kernel", task, "bmod");

						/* this is the last fwd */
						raw_edge_2d(&lastwrite, ii, kk, g, task);

						/* this is the last bdiv */
						raw_edge_2d(&lastwrite, kk, jj, g, task);

						raw_edge_2d(&lastwrite, ii, jj, g, task);
						MATRIX(lastwrite, ii, jj) = task;
					}
	}
	ggen_error_clean(1);
	return GGEN_SUCCESS;
ggen_error_label:
	return GGEN_FAILURE;

}

/* SparseLU: generate the graph from a LU decomposition of a sparse matrix of
 * size size*size BLOCKS.
 */
igraph_t *ggen_generate_sparselu(unsigned long size)
{
	igraph_matrix_bool_t nonempty;
	igraph_t *g;

	ggen_error_start_stack();
	g = malloc(sizeof(igraph_t));
	GGEN_CHECK_ALLOC(g);
	GGEN_FINALLY3(free,g,1);

	GGEN_CHECK_IGRAPH(igraph_matrix_bool_init(&nonempty, size, size));
	GGEN_FINALLY(igraph_matrix_destroy, &nonempty);

	/* start by figuring out which part of the matrix contains elements
	 */
	sparselu_genmat(&nonempty, size);

	GGEN_CHECK_INTERNAL(generate_lu(g, size, nonempty));
	ggen_error_clean(1);
	return g;
ggen_error_label:
	return NULL;
}

/* DenseLU: generate the graph from a LU decomposition of a dense matrix of
 * size size*size BLOCKS.
 */
igraph_t *ggen_generate_denselu(unsigned long size)
{
	igraph_matrix_bool_t nonempty;
	igraph_t *g;

	ggen_error_start_stack();
	g = malloc(sizeof(igraph_t));
	GGEN_CHECK_ALLOC(g);
	GGEN_FINALLY3(free,g,1);

	GGEN_CHECK_IGRAPH(igraph_matrix_bool_init(&nonempty, size, size));
	GGEN_FINALLY(igraph_matrix_destroy, &nonempty);

	igraph_matrix_bool_fill(&nonempty, 1);

	GGEN_CHECK_INTERNAL(generate_lu(g, size, nonempty));
	ggen_error_clean(1);
	return g;
ggen_error_label:
	return NULL;
}


/* Poisson2D: iterations of a "sweep" across the unit square cut into a grid
 * of n by n evenly-spaced points.
 */
igraph_t *ggen_generate_poisson2d(unsigned long n, unsigned long iter)
{
	igraph_t *g = NULL;
	igraph_vector_long_t lw_old, lw_new;
	unsigned long it, i, task, from;

	ggen_error_start_stack();

	g = malloc(sizeof(igraph_t));
	GGEN_CHECK_ALLOC(g);
	GGEN_FINALLY3(free,g,1);

	GGEN_CHECK_IGRAPH(igraph_empty(g,0,1));
	GGEN_FINALLY3(igraph_destroy,g,1);

	/* We mimic a sweep across 2 arrays. Each iteration is doing:
	 * - old[i] = new[i]
	 * - new[i] = f(old[i-1], old[i], old[i+1])
	 * To figure out dependencies between tasks, lw_old stores the last
	 * task that wrote into old, and the same for lw_new/new.
	 * Border cells never change values, so we avoid creating tasks for
	 * them.
	 */
	GGEN_CHECK_IGRAPH(igraph_vector_long_init(&lw_old, n));
	GGEN_FINALLY(igraph_vector_destroy, &lw_old);
	igraph_vector_long_fill(&lw_old, -1);
	GGEN_CHECK_IGRAPH(igraph_vector_long_init(&lw_new, n));
	GGEN_FINALLY(igraph_vector_destroy, &lw_new);
	igraph_vector_long_fill(&lw_new, -1);

	for(it = 0; it < iter; it++)
	{
		for(i = 1; i < n-1; i++)
		{
			/* old[i] = new[i] */
			task = addtask(g);
			SETVAS(g, "kernel", task, "copy");
			raw_edge_1d(&lw_new, i, g, task);
			VECTOR(lw_old)[i] = task;
		}
		for(i = 1; i < n-1; i++)
		{
			/* new[i] = f(old[i-1], old[i], old[i+1])
			 * Note that on the edges, the dependencies are simpler
			 */
			task = addtask(g);
			SETVAS(g, "kernel", task, "apply");
			raw_edge_1d(&lw_old, i-1, g, task);
			raw_edge_1d(&lw_old, i  , g, task);
			raw_edge_1d(&lw_old, i+1, g, task);
			VECTOR(lw_new)[i] = task;
		}
	}
	ggen_error_clean(1);
	return g;
ggen_error_label:
	return NULL;
}
