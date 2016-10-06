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
 * To manage that, we use long vector remembering the last task that wrote into
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
	if(todo != -1)
	{
		from = VECTOR(*lastwrite)[i];
		igraph_add_edge(g, from, task);
	}
}

static inline void raw_edge_2d(igraph_matrix_long_t *lastwrite,
			       unsigned long i, unsigned long j,
			       igraph_t *g, unsigned long task)
{
	long todo = MATRIX(*lastwrite,i,j);
	unsigned long from;
	if(todo != -1)
	{
		from = MATRIX(*lastwrite,i,j);
		igraph_add_edge(g, from, task);
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

	/* There's a single matrix used inout, use lastwrite to create the right
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

/* creates two subtasks, add the right edges to the graph */
int _fibonacci_add_tasks(unsigned long n, unsigned long cutoff,
			 unsigned long myid, igraph_t *g)
{
	unsigned long lastid;
	SETVAN(g, "n", myid, n);
	/* this task has already been created, or we got to the bottom */
	if(n < 2 || n <= cutoff)
		return GGEN_SUCCESS;
	else
	{
		/* create our two subtasks, and link them to me
		 * we need to know the current number of vertices, to identify
		 * which vertices id the new ones will have.
		 */
		lastid = igraph_vcount(g);
		igraph_add_vertices(g,2,NULL);
		igraph_add_edge(g, lastid, myid);
		igraph_add_edge(g, lastid+1, myid);
		_fibonacci_add_tasks(n-1, cutoff, lastid, g);
		_fibonacci_add_tasks(n-2, cutoff, lastid+1, g);
	}
	return GGEN_SUCCESS;
}


/* Fibonacci: generate the graph of the typical, recursive fibo(n)
 *
 * Customizable cutoff (value after which the algorithm is sequential).
 * This is the 3 tasks per level version, i.e the final add is also a task.
 */
igraph_t *ggen_generate_fibonacci(unsigned long n, unsigned long cutoff)
{
	igraph_t *g = NULL;

	ggen_error_start_stack();
	if(cutoff > n)
		GGEN_SET_ERRNO(GGEN_EINVAL);

	g = malloc(sizeof(igraph_t));
	GGEN_CHECK_ALLOC(g);
	GGEN_FINALLY3(free,g,1);

	/* recursive creation: create a task, call the function, it will create
	 * the right edges itself.
	 */
	GGEN_CHECK_IGRAPH(igraph_empty(g,1,1));
	GGEN_FINALLY3(igraph_destroy,g,1);
	GGEN_CHECK_INTERNAL(_fibonacci_add_tasks(n, cutoff, 0, g));
	ggen_error_clean(1);
	return g;
ggen_error_label:
	return NULL;
}

/* fork-join: generate a graph of multiple phases of fork-joins. All phases have
 * the same diameter (number of forks).
 */
igraph_t *ggen_generate_forkjoin(unsigned long phases, unsigned long diameter)
{
	igraph_t *g = NULL;
	igraph_vector_t edges;
	unsigned long numvertices, source, sink, i;

	ggen_error_start_stack();

	g = malloc(sizeof(igraph_t));
	GGEN_CHECK_ALLOC(g);
	GGEN_FINALLY3(free,g,1);

	/* number of tasks: 1 source, for each phase diameter forks and 1 join.
	 */
	numvertices = 1 + (diameter+1)*phases;
	GGEN_CHECK_IGRAPH(igraph_empty(g,numvertices,1));
	GGEN_FINALLY3(igraph_destroy,g,1);

	GGEN_CHECK_IGRAPH(igraph_vector_init(&edges, 4*diameter));

	/* iterate over the phases, adding the right edges at the time.
	 * We stop once source points to the last sink.
	 */
	for(source = 0; source < numvertices-1; source += diameter+1)
	{
		sink = source + diameter +1;
		for(i = 0; i < diameter; i++)
		{
			VECTOR(edges)[4*i] = source;
			VECTOR(edges)[4*i+1] = source + i +1;
			VECTOR(edges)[4*i+2] = source + i +1;
			VECTOR(edges)[4*i+3] = sink;
		}
		GGEN_CHECK_IGRAPH(igraph_add_edges(g, &edges, NULL));
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

/* SparseLU: generate the graph from a LU decomposition of a sparse matrix of
 * size size*size BLOCKS.
 */
igraph_t *ggen_generate_sparselu(unsigned long size)
{
	igraph_t *g = NULL;
	igraph_matrix_bool_t nonempty;
	igraph_matrix_long_t lastwrite;
	unsigned long ii, jj, kk, to, from, task, lastlu;

	ggen_error_start_stack();

	g = malloc(sizeof(igraph_t));
	GGEN_CHECK_ALLOC(g);
	GGEN_FINALLY3(free,g,1);

	GGEN_CHECK_IGRAPH(igraph_empty(g,0,1));
	GGEN_FINALLY3(igraph_destroy,g,1);
	GGEN_CHECK_IGRAPH(igraph_matrix_bool_init(&nonempty, size, size));
	GGEN_FINALLY(igraph_matrix_destroy, &nonempty);
	GGEN_CHECK_IGRAPH(igraph_matrix_long_init(&lastwrite, size, size));
	GGEN_FINALLY(igraph_matrix_destroy, &lastwrite);
	igraph_matrix_long_fill(&lastwrite, -1);

	/* start by figuring out which part of the matrix contains elements
	 */
	sparselu_genmat(&nonempty, size);

	/* run through the motions of the algorithm, creating tasks
	 * and checking last writers.
	 */
	for(kk = 0; kk < size; kk++)
	{
		/* lu task: inout [kk*size +kk]*/
		lastlu = addtask(g);
		SETVAS(g, "kernel", lastlu, "lu");
		SETVAN(g, "x", lastlu, kk);
		SETVAN(g, "y", lastlu, kk);
		raw_edge_2d(&lastwrite, kk, kk, g, lastlu);
		MATRIX(lastwrite, kk, kk) = lastlu;

		for(jj = kk+1; jj < size; jj++)
			if(MATRIX(nonempty, kk, jj))
			{
				/* fwd: in [kk*size + kk] inout [kk*size+jj] */
				task = addtask(g);
				SETVAS(g, "kernel", task, "fwd");
				SETVAN(g, "x", task, kk);
				SETVAN(g, "y", task, jj);
				igraph_add_edge(g, lastlu, task);
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
				SETVAN(g, "x", task, ii);
				SETVAN(g, "y", task, kk);
				igraph_add_edge(g, lastlu, task);
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
						SETVAN(g, "x", task, ii);
						SETVAN(g, "y", task, jj);

						/* this is the last fwd */
						from = MATRIX(lastwrite, ii, kk);
						igraph_add_edge(g, from, task);

						/* this is the last bdiv */
						from = MATRIX(lastwrite, kk, jj);
						igraph_add_edge(g, from, task);

						raw_edge_2d(&lastwrite, ii, jj, g, task);
						MATRIX(lastwrite, ii, jj) = task;
					}
	}
	ggen_error_clean(1);
	return g;
ggen_error_label:
	return NULL;

}

/* recursive part of strassen:
 * takes as input the last tasks to touch a and b.
 */
long _strassen(unsigned long mytask, unsigned long size,
	      unsigned long depth, unsigned long cutdepth, unsigned long cutseq,
	      igraph_t *g)
{
	unsigned long quadrant = size >> 1;
	unsigned long c, c12, c21, c22;
	unsigned long s1, s2, s3, s4, s5, s6, s7, s8, m2, m5, t1, task;

	SETVAS(g, "kernel", mytask, "strassen");
	if(size <= cutseq || depth >= cutdepth)
	{
		return mytask;
	}

	/* S1 compute: in a21 a22, out:s1 */
	s1 = addtask(g);
	SETVAS(g, "kernel", s1, "s1");
	igraph_add_edge(g, mytask, s1);

	/* S2: in: s1  a, out: s2 */
	s2 = addtask(g);
	SETVAS(g, "kernel", s2, "s2");
	igraph_add_edge(g, s1, s2);
	igraph_add_edge(g, mytask, s2);

	/* S4: in: a12 s2, out: s4 */
	s4 = addtask(g);
	SETVAS(g, "kernel", s4, "s4");
	igraph_add_edge(g, s2, s4);
	igraph_add_edge(g, mytask, s4);

	/* S5: in: b12 b out: s5 */
	s5 = addtask(g);
	SETVAS(g, "kernel", s5, "s5");
	igraph_add_edge(g, mytask, s5);

	/* S6: in: b22 s5 out: s6 */
	s6 = addtask(g);
	SETVAS(g, "kernel", s6, "s6");
	igraph_add_edge(g, s5, s6);
	igraph_add_edge(g, mytask, s6);

	/* S8: in: s6 b21 out: s8 */
	s8 = addtask(g);
	SETVAS(g, "kernel", s8, "s8");
	igraph_add_edge(g, s6, s8);
	igraph_add_edge(g, mytask, s8);

	/* S3: in: a a21 out: s3 */
	s3 = addtask(g);
	SETVAS(g, "kernel", s3, "s3");
	igraph_add_edge(g, mytask, s3);

	/* S7: in: b22 b12 out: s7 */
	s7 = addtask(g);
	SETVAS(g, "kernel", s7, "s7");
	igraph_add_edge(g, mytask, s7);

	/* M2: in a, b out: m2 */
	task = addtask(g);
	SETVAS(g, "kernel", task, "m2");
	igraph_add_edge(g, mytask, task);
	m2 = _strassen(task, quadrant, depth+1, cutdepth, cutseq, g);

	/* M5: in: s1 s5 out: m5 */
	task = addtask(g);
	SETVAS(g, "kernel", task, "m5");
	igraph_add_edge(g, s1, task);
	igraph_add_edge(g, s5, task);
	m5 = _strassen(task, quadrant, depth+1, cutdepth, cutseq, g);

	/* t1: in s2 s6 out: t1 */
	task = addtask(g);
	SETVAS(g, "kernel", task, "t1");
	igraph_add_edge(g, s2, task);
	igraph_add_edge(g, s6, task);
	t1 = _strassen(task, quadrant, depth+1, cutdepth, cutseq, g);

	/* C22: in: s3 s7 out: c22 */
	task = addtask(g);
	SETVAS(g, "kernel", task, "c22");
	igraph_add_edge(g, s3, task);
	igraph_add_edge(g, s7, task);
	c22 = _strassen(task, quadrant, depth+1, cutdepth, cutseq, g);

	/* C: in: a12 b21 out: c */
	task = addtask(g);
	SETVAS(g, "kernel", task, "c");
	igraph_add_edge(g, mytask, task);
	c = _strassen(task, quadrant, depth+1, cutdepth, cutseq, g);

	/* C12: in: s4 b22 out: c12 */
	task = addtask(g);
	SETVAS(g, "kernel", task, "c12");
	igraph_add_edge(g, s4, task);
	igraph_add_edge(g, mytask, task);
	c12 = _strassen(task, quadrant, depth+1, cutdepth, cutseq, g);

	/* C21: in: a22 s8 out: c21 */
	task = addtask(g);
	SETVAS(g, "kernel", task, "c21");
	igraph_add_edge(g, mytask, task);
	igraph_add_edge(g, s8, task);
	c21 = _strassen(task, quadrant, depth+1, cutdepth, cutseq, g);

	/* C: inout: C in: m2 */
	task = addtask(g);
	SETVAS(g, "kernel", task, "c_inout");
	igraph_add_edge(g, c, task);
	igraph_add_edge(g, m2, task);
	c = task;

	/* C12: inout: c12 in: m5 t1 m2 */
	task = addtask(g);
	SETVAS(g, "kernel", task, "c12_inout");
	igraph_add_edge(g, m5, task);
	igraph_add_edge(g, t1, task);
	igraph_add_edge(g, m2, task);
	igraph_add_edge(g, c12, task);
	c12 = task;

	/* C21: inout: c21 in: c22 t1 m2 */
	task = addtask(g);
	SETVAS(g, "kernel", task, "c21_inout");
	igraph_add_edge(g, c22, task);
	igraph_add_edge(g, t1, task);
	igraph_add_edge(g, m2, task);
	igraph_add_edge(g, c21, task);
	c21 = task;

	/* C22: inout: c22 in: m5 t1 m2 */
	task = addtask(g);
	SETVAS(g, "kernel", task, "c22_inout");
	igraph_add_edge(g, m5, task);
	igraph_add_edge(g, t1, task);
	igraph_add_edge(g, m2, task);
	igraph_add_edge(g, c22, task);
	c22 = task;

	/* taskwait: c c12 c21 c22 */
	task = addtask(g);
	SETVAS(g, "kernel", task, "taskwait");
	igraph_add_edge(g, c, task);
	igraph_add_edge(g, c12, task);
	igraph_add_edge(g, c21, task);
	igraph_add_edge(g, c22, task);
	return task;
}

/* Strassen: efficient matrix multiply, of size n*n.
 * The code has two cutoffs points: recurse depth and sequential cutoff (the
 * submatrix is so small that sequential exec is faster).
 * Tasks dependencies are lifted from KaStORS/BOTS.
 */
igraph_t *ggen_generate_strassen(unsigned long n, unsigned long depth,
				 unsigned long seq)
{
	igraph_t *g = NULL;

	ggen_error_start_stack();

	g = malloc(sizeof(igraph_t));
	GGEN_CHECK_ALLOC(g);
	GGEN_FINALLY3(free,g,1);

	GGEN_CHECK_IGRAPH(igraph_empty(g,1,1));
	GGEN_FINALLY3(igraph_destroy,g,1);

	_strassen(0, n, 1, depth, seq, g);
	
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
