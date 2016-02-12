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

static inline void raw_edge_2d(igraph_vector_long_t *lastwrite,
			       unsigned long i, unsigned long j,
			       unsigned long size, igraph_t *g,
			       unsigned long task)
{
	long todo = VECTOR(*lastwrite)[i*size +j];
	unsigned long from;
	if(todo != -1)
	{
		from = VECTOR(*lastwrite)[i*size +j];
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
	igraph_vector_long_t lastwrite;
	unsigned long k, m, n, task;

	ggen_error_start_stack();

	g = malloc(sizeof(igraph_t));
	GGEN_CHECK_ALLOC(g);
	GGEN_FINALLY3(free,g,1);

	GGEN_CHECK_IGRAPH(igraph_empty(g,0,1));
	GGEN_FINALLY3(igraph_destroy,g,1);

	GGEN_CHECK_IGRAPH(igraph_vector_long_init(&lastwrite, size*size));
	GGEN_FINALLY(igraph_vector_destroy, &lastwrite);
	igraph_vector_long_fill(&lastwrite, -1);

	/* run through the motions of the algorithm, creating tasks and checking
	 * their dependencies as we go. Since all tasks touch the same memory at
	 * different times, we just track the last task (in creation order),
	 * that writes to each block.
	 * -1 in last write is used to identify unwritten blocks.
	 */
	for(k = 0; k < size; k++)
	{
		/* potrf inout [k,k]*/
		task = addtask(g);
		raw_edge_2d(&lastwrite, k, k, size, g, task);
		VECTOR(lastwrite)[k*size + k] = task;

		for(m = k+1; m < size; m++)
		{
			/* trsm: in [k,k] inout [k,m] */
			task = addtask(g);
			raw_edge_2d(&lastwrite, k, k, size, g, task);
			raw_edge_2d(&lastwrite, k, m, size, g, task);
			VECTOR(lastwrite)[k*size +m] = task;
		}
		for(m = k+1; m < size; m++)
		{
			/* syrk: in [k,m] inout [m,m] */
			task = addtask(g);
			raw_edge_2d(&lastwrite, k, m, size, g, task);
			raw_edge_2d(&lastwrite, m, m, size, g, task);
			VECTOR(lastwrite)[m*size +m] = task;

			for(n = k+1; n < m; n++)
			{
				/* gemm: in [k,n] in [k,m] inout [n,m] */
				task = addtask(g);
				raw_edge_2d(&lastwrite, k, n, size, g, task);
				raw_edge_2d(&lastwrite, k, m, size, g, task);
				raw_edge_2d(&lastwrite, n, m, size, g, task);
				VECTOR(lastwrite)[m*size +m] = task;
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
static int sparselu_genmat(igraph_vector_bool_t *matrix, unsigned long size)
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
			VECTOR(*matrix)[ii*size +jj] = !empty;
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
	igraph_vector_bool_t nonempty;
	igraph_vector_long_t lastwrite;
	unsigned long ii, jj, kk, to, from, task, lastlu;

	ggen_error_start_stack();

	g = malloc(sizeof(igraph_t));
	GGEN_CHECK_ALLOC(g);
	GGEN_FINALLY3(free,g,1);

	GGEN_CHECK_IGRAPH(igraph_empty(g,0,1));
	GGEN_FINALLY3(igraph_destroy,g,1);
	GGEN_CHECK_IGRAPH(igraph_vector_bool_init(&nonempty, size*size));
	GGEN_FINALLY(igraph_vector_destroy, &nonempty);
	GGEN_CHECK_IGRAPH(igraph_vector_long_init(&lastwrite, size*size));
	GGEN_FINALLY(igraph_vector_destroy, &lastwrite);

	/* start by figuring out which part of the matrix contains elements
	 */
	sparselu_genmat(&nonempty, size);

	/* run through the motions of the algorithm, creating tasks and checking
	 * their dependencies as we go. Since all tasks touch the same memory at
	 * different times, we just track the last task (in creation order),
	 * that writes to each block.
	 * -1 in last write is used to identify unwritten blocks.
	 */
	igraph_vector_long_fill(&lastwrite, -1);
	for(kk = 0; kk < size; kk++)
	{
		/* lu task: inout [kk*size +kk]*/
		lastlu = addtask(g);
		SETVAS(g, "kernel", lastlu, "lu");
		SETVAN(g, "x", lastlu, kk);
		SETVAN(g, "y", lastlu, kk);
		if(VECTOR(lastwrite)[kk*size +kk] != -1)
		{
			from = VECTOR(lastwrite)[kk*size +kk];
			igraph_add_edge(g, from, lastlu);
		}
		VECTOR(lastwrite)[kk*size + kk] = lastlu;

		for(jj = kk+1; jj < size; jj++)
			if(VECTOR(nonempty)[kk*size +jj])
			{
				/* fwd: in [kk*size + kk] inout [kk*size+jj] */
				task = addtask(g);
				SETVAS(g, "kernel", task, "fwd");
				SETVAN(g, "x", task, kk);
				SETVAN(g, "y", task, jj);
				igraph_add_edge(g, lastlu, task);
				if(VECTOR(lastwrite)[kk*size +jj] != -1)
				{
					from = VECTOR(lastwrite)[kk*size+jj];
					igraph_add_edge(g, from, task);
				}
				VECTOR(lastwrite)[kk*size+jj] = task;
			}
		for(ii = kk+1; ii < size; ii++)
			if(VECTOR(nonempty)[ii*size +kk])
			{
				/* bdiv: in [kk*size +kk] inout: [ii*size +kk]
				 */
				task = addtask(g);
				SETVAS(g, "kernel", task, "bdiv");
				SETVAN(g, "x", task, ii);
				SETVAN(g, "y", task, kk);
				igraph_add_edge(g, lastlu, task);
				if(VECTOR(lastwrite)[ii*size +kk] != -1)
				{
					from = VECTOR(lastwrite)[ii*size+kk];
					igraph_add_edge(g, from, task);
				}
				VECTOR(lastwrite)[ii*size+kk] = task;
			}
		for(ii = kk+1; ii < size; ii++)
			if(VECTOR(nonempty)[ii*size +kk])
				for(jj = kk+1; jj < size; jj++)
					if(VECTOR(nonempty)[kk*size +jj])
					{
						/* bmod: in [ii*size +kk]
						 *       in [kk*size +jj]
						 *       inout [ii*size +jj]
						 */
						VECTOR(nonempty)[ii*size +jj] = 1;
						task = addtask(g);
						SETVAS(g, "kernel", task, "bmod");
						SETVAN(g, "x", task, ii);
						SETVAN(g, "y", task, jj);

						/* this is the last fwd */
						from = VECTOR(lastwrite)[ii*size+kk];
						igraph_add_edge(g, from, task);

						/* this is the last bdiv */
						from = VECTOR(lastwrite)[kk*size+jj];
						igraph_add_edge(g, from, task);

						if(VECTOR(lastwrite)[ii*size+jj] != -1)
						{
							from = VECTOR(lastwrite)[ii*size+jj];
							igraph_add_edge(g, from, task);
						}
						VECTOR(lastwrite)[ii*size+jj] = task;
					}
	}
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
	 */
	GGEN_CHECK_IGRAPH(igraph_vector_long_init(&lw_old, n));
	GGEN_FINALLY(igraph_vector_destroy, &lw_old);
	GGEN_CHECK_IGRAPH(igraph_vector_long_init(&lw_new, n));
	GGEN_FINALLY(igraph_vector_destroy, &lw_new);

	/* -1 means no task wrote to this index yet. */
	igraph_vector_long_fill(&lw_old, -1);
	igraph_vector_long_fill(&lw_new, -1);

	for(it = 0; it < iter; it++)
	{
		for(i = 0; i < n; i++)
		{
			/* old[i] = new[i] */
			task = addtask(g);
			if(VECTOR(lw_new)[i] != -1)
			{
				from = VECTOR(lw_new)[i];
				igraph_add_edge(g, from, task);
			}
			VECTOR(lw_old)[i] = task;
		}
		for(i = 0; i < n; i++)
		{
			/* new[i] = f(old[i-1], old[i], old[i+1])
			 * Note that on the edges, the dependencies are simpler
			 */
			task = addtask(g);
			if(i > 0 && i < n-1)
			{
				from = VECTOR(lw_old)[i-1];
				igraph_add_edge(g, from, task);
				from = VECTOR(lw_old)[i];
				igraph_add_edge(g, from, task);
				from = VECTOR(lw_old)[i+1];
				igraph_add_edge(g, from, task);
			}
			VECTOR(lw_new)[i] = task;
		}
	}
	ggen_error_clean(1);
	return g;
ggen_error_label:
	return NULL;
}
