/* Copyright Swann Perarnau 2009
*
*   contact : Swann.Perarnau@imag.fr
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

#ifndef UTILS_H
#define UTILS_H

#include "ggen.h"

/* gsl rng stuff */
int ggen_rng_init(gsl_rng **r);
int ggen_rng_save(gsl_rng **r,const char *file);
int ggen_rng_load(gsl_rng **r,const char *file);

/* string conversion */
int s2ul(char *s,unsigned long *l);
int s2d(char *s,double *d);

/* graph io */
#define GGEN_GRAPH_NAME_ATTR "__ggen_graph_name"
#define GGEN_DEFAULT_GRAPH_NAME "dag"
#define GGEN_VERTEX_NAME_ATTR "__ggen_vname"
#define GGEN_DEFAULT_NAME_SIZE 80
int ggen_read_graph(igraph_t *g,FILE *input);

int ggen_write_graph(igraph_t *g,FILE *output);

#endif
