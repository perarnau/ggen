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

#ifndef GGEN_H
#define GGEN_H 1

/* igraph is used for graph manipulation */
#include<igraph/igraph.h>
/* GNU Scientific Library provides random number generators */
#include<gsl/gsl_rng.h>
#include<gsl/gsl_randist.h>

/**********************************************************
 * Error reporting
 *********************************************************/

const char* ggen_error_strerror(void);

/**********************************************************
 * Analysis methods
 *********************************************************/

igraph_vector_t * ggen_analyze_longest_path(igraph_t *g);

igraph_vector_t * ggen_analyze_longest_antichain(igraph_t *g);

igraph_vector_t * ggen_analyze_lowest_single_ancestor(igraph_t *g);

igraph_vector_t * ggen_analyze_edge_disjoint_paths(igraph_t *g);

/**********************************************************
 * Generation methods
 *********************************************************/

igraph_t *ggen_generate_erdos_gnm(gsl_rng *r, unsigned long n, unsigned long m);

igraph_t *ggen_generate_erdos_gnp(gsl_rng *r, unsigned long n, double p);

igraph_t *ggen_generate_erdos_lbl(gsl_rng *r, unsigned long n, double p, unsigned long nbl);

igraph_t *ggen_generate_fifo(gsl_rng *r, unsigned long n, unsigned long od, unsigned long id);

igraph_t *ggen_generate_random_orders(gsl_rng *r, unsigned long n, unsigned int orders);

/**********************************************************
 * Static graphs
 *********************************************************/

igraph_t *ggen_generate_fibonacci(unsigned long n, unsigned long cutoff);

igraph_t *ggen_generate_forkjoin(unsigned long phases, unsigned long diameter);

igraph_t *ggen_generate_sparselu(unsigned long size);

/**********************************************************
 * Transformation methods
 *********************************************************/

enum ggen_transform_t { GGEN_TRANSFORM_SOURCE, GGEN_TRANSFORM_SINK };

int ggen_transform_add(igraph_t *g, enum ggen_transform_t t);

int ggen_transform_delete(igraph_t *g, enum ggen_transform_t t);

int ggen_transform_transitive_closure(igraph_t *g);


/**********************************************************
 * IO methods
 *********************************************************/

/* default values for graph attributes */
#define GGEN_GRAPH_NAME_ATTR "__ggen_graph_name"
#define GGEN_DEFAULT_GRAPH_NAME "dag"
#define GGEN_VERTEX_NAME_ATTR "__ggen_vname"
#define GGEN_DEFAULT_NAME_SIZE 80
int ggen_read_graph(igraph_t *g,FILE *input);

int ggen_write_graph(igraph_t *g,FILE *output);

/* get vertex name:
 * if name exists in graph, will return a pointer to it.
 * if not, will write at most GGEN_DEFAULT_NAME_SIZE into buf
 */
char * ggen_vname(igraph_t *g, char *buf, unsigned long id);

#endif // GGEN_H
