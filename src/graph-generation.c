/* Copyright Swann Perarnau 2009
*
*   contributor(s) :
*	Pradeep Beniwal (pdpbeniwal AT gmail.com)
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

/* Erdos-Renyi : G(n,M)
*/
igraph_t *ggen_generate_erdos_gnm(gsl_rng *r, unsigned long n, unsigned long m)
{
	igraph_matrix_t adj;
	igraph_t *g = NULL;
	int err;
	unsigned long i,j;
	unsigned long added_edges = 0;

	if(r == NULL)
		return NULL;

	if(m > (n*(n-1)/2))
		return NULL;

	g = malloc(sizeof(igraph_t));
	if(g == NULL)
		return NULL;

	if(m == 0 || n <= 1)
	{
		err = igraph_empty(g,n,1);
		if(err) goto error;
		else return g;
	}
	if(m == (n*(n-1))/2)
	{
		err = igraph_full_citation(g,n,1);
		if(err) goto error;
		else return g;

	}

	err = igraph_matrix_init(&adj,n,n);
	if(err)	goto error;

	igraph_matrix_null(&adj);

	while(added_edges < m) {
		i = gsl_rng_uniform_int(r,n);
		j = gsl_rng_uniform_int(r,n);

                if(i < j && igraph_matrix_e(&adj,i,j) == 0)
                {
			igraph_matrix_set(&adj,i,j,1);
			added_edges++;
                }

        }

	err = igraph_adjacency(g,&adj,IGRAPH_ADJ_DIRECTED);
	igraph_matrix_destroy(&adj);
	if(err) goto error;

	goto ret;
error:
	free(g);
	g = NULL;
ret:
	return g;
}

/* Erdos-Renyi : G(n,p)
*/
igraph_t *ggen_generate_erdos_gnp(gsl_rng *r, unsigned long n, double p)
{
	igraph_matrix_t m;
	igraph_t *g = NULL;
	int err;
	unsigned long i,j;

	if(r == NULL)
		return NULL;

	if(p < 0.0 || p > 1.0)
		return NULL;

	g = malloc(sizeof(igraph_t));
	if(g == NULL)
		return NULL;

	if(p == 0.0)
	{
		err = igraph_empty(g,n,1);
		if(err) goto error;
		else return g;
	}
	if(p == 1.0)
	{
		err = igraph_full_citation(g,n,1);
		if(err) goto error;
		else return g;
	}

	err = igraph_matrix_init(&m,n,n);
	if(err) goto error;

	for(i = 0; i < n; i++)
		for(j = 0; j < n; j++)
			if(i < j)
				// coin flipping to determine if we add an edge or not
				igraph_matrix_set(&m,i,j,gsl_ran_bernoulli(r,p));
			else
				igraph_matrix_set(&m,i,j,0);

	err = igraph_adjacency(g,&m,IGRAPH_ADJ_DIRECTED);
	igraph_matrix_destroy(&m);
	if(err) goto error;

	goto ret;

error:
	free(g);
	g = NULL;
ret:
	return g;
}

igraph_t *ggen_generate_erdos_lbl(gsl_rng *r, unsigned long n, double p, unsigned long nbl)
{
	igraph_t *g = NULL;
	igraph_matrix_t m;
	igraph_vector_t layers;
	unsigned long i,j;
	int err;

	if(r == NULL)
		return NULL;

	if(p < 0.0 || p > 1.0)
		return NULL;

	if(nbl > n || nbl == 0)
		return NULL;

	g = malloc(sizeof(igraph_t));
	if(g == NULL)
		return NULL;

	if(p == 0.0)
	{
		err = igraph_empty(g,n,1);
		if(err) goto error;
		else return g;
	}
	if(p == 1.0 && nbl == n)
	{
		err = igraph_full_citation(g,n,1);
		if(err) goto error;
		else return g;
	}

	err = igraph_matrix_init(&m,n,n);
	if(err) goto error;

	err = igraph_vector_init(&layers,n);
	if(err)
		goto d_m;

	// asign to each vertex a layer
	for(i = 0; i < n; i++)
	{
		j = gsl_rng_uniform_int(r,nbl);
		VECTOR(layers)[i] = j;
	}

	// create edges
	for(i = 0; i < n; i++)
		for(j = 0; j < n; j++)
			// if the layer allocation allows the edge, we test for it
			if(VECTOR(layers)[i]<VECTOR(layers)[j])
				igraph_matrix_set(&m,i,j,gsl_ran_bernoulli(r,p));
			else
				igraph_matrix_set(&m,i,j,0);

	//translate the matrix to a graph
	err = igraph_adjacency(g,&m,IGRAPH_ADJ_DIRECTED);
	igraph_vector_destroy(&layers);
d_m:
	igraph_matrix_destroy(&m);
	if(err) goto error;

	goto ret;
error:
	free(g);
	g = NULL;
ret:
	return g;
}

/* Fan-in/ Fan-out method
*/
igraph_t *ggen_generate_fifo(gsl_rng *r, unsigned long n, unsigned long od, unsigned long id)
{
	igraph_t *g = NULL;
	igraph_vector_t available_od;
	igraph_vector_t out_degrees;
	igraph_vector_t vertices;
	igraph_vector_t choice;
	igraph_vector_t edges;
	unsigned long max;
	unsigned long i,j,k;
	unsigned long vcount = 1;
	int err;

	if(r == NULL)
		return NULL;

	if(id == 0 || od == 0 || od > n || id > n)
		return NULL;

	g = malloc(sizeof(igraph_t));
	if(g == NULL)
		return NULL;

	err = igraph_empty(g,1,1);
	if(err)
	{
		free(g);
		return NULL;
	}

	while(vcount < n)
	{
		// some structs can be initialised together
		err = igraph_vector_init(&available_od,vcount);
		if(err) goto error;

		err = igraph_vector_init(&out_degrees,vcount);
		if(err)	goto error_io;

		err = igraph_vector_init(&vertices,vcount);
		if(err) goto error_iv;

		// compute the available out degree of each vertex
		err = igraph_degree(g,&out_degrees,igraph_vss_all(),IGRAPH_OUT,0);
		if(err) goto error_common;

		// fill available with od and substract out_degrees
		igraph_vector_fill(&available_od,od);
		err = igraph_vector_sub(&available_od,&out_degrees);
		if(err) goto error_common;

		if(gsl_ran_bernoulli(r,0.5))     //Fan-out Step
		{
			// find max
			max = igraph_vector_max(&available_od);

			// register all vertices having max as outdegree
			j = 0;
			for (i = 0; i < vcount;i++)
				if(VECTOR(available_od)[i] == max)
					VECTOR(vertices)[j++] = i;

			// choose randomly a vertex among availables
			i = gsl_rng_uniform_int(r,j);

			// how many children ?
			j = gsl_rng_uniform_int(r,max) +1;

			// create all new nodes and add edges
			err = igraph_add_vertices(g,j,NULL);
			if(err)	goto error_common;

			err = igraph_vector_init(&edges,j*2);
			if(err)	goto error_common;

			for(k = 0; k < j; k++)
			{
				VECTOR(edges)[2*k] = i;
				VECTOR(edges)[2*k+1] = vcount + k;
			}
			vcount+=k;
		}
		else	//Fan-In Step
		{
			// register all vertices having an available outdegree
			j = 0;
			for (i = 0; i < vcount;i++)
				if(VECTOR(available_od)[i] > 0)
					VECTOR(vertices)[j++] = i;

			// we can add at most id vertices
			max =( j > id)? id: j;
			// how many edges to add
			k = gsl_rng_uniform_int(r,max) +1;

			// choose that many nodes and add edges from them to the new node
			err = igraph_vector_init(&choice,k);
			if(err)	goto error_common;

			gsl_ran_choose(r,VECTOR(choice),k,
				VECTOR(vertices),j,sizeof(VECTOR(vertices)[0]));

			// add a vertex to the graph
			err = igraph_add_vertices(g,1,NULL);
			if(err)
			{
				igraph_vector_destroy(&choice);
				goto error_common;
			}

			err = igraph_vector_init(&edges,k*2);
			if(err)
			{
				igraph_vector_destroy(&choice);
				goto error_common;
			}
			// be carefull, vcount is the last ID of vertices not vcount +1
			for(i = 0; i < k; i++)
			{
				VECTOR(edges)[2*i] = VECTOR(choice)[i];
				VECTOR(edges)[2*i+1] = vcount;
			}
			vcount++;
			igraph_vector_destroy(&choice);

		}
		// in all cases, edges should be added
		err = igraph_add_edges(g,&edges,NULL);
		igraph_vector_destroy(&edges);
		if(err) goto error_common;

		// destroy common vectors
		igraph_vector_destroy(&vertices);
		igraph_vector_destroy(&out_degrees);
		igraph_vector_destroy(&available_od);
	}
	return g;
error_common:
	igraph_vector_destroy(&vertices);
error_iv:
	igraph_vector_destroy(&out_degrees);
error_io:
	igraph_vector_destroy(&available_od);
error:
	igraph_destroy(g);
	free(g);
	return NULL;
}


/* Random Orders Method :
*/
igraph_t * ggen_generate_random_orders(gsl_rng *r, unsigned long n, unsigned int orders)
{
	igraph_t *g = NULL;
	igraph_matrix_t m,edge_validity;
	int err = 0;
	long long i = 0,j,k;
	igraph_vector_t posets[orders];
	igraph_vector_t index[orders];

	if(r == NULL)
		return NULL;

	if(orders == 0)
		return NULL;

	// init structures
	g = malloc(sizeof(igraph_t));
	if(g == NULL)
		return NULL;

	err = igraph_matrix_init(&m,n,n);
	if(err)
		goto f_g;

	err = igraph_matrix_init(&edge_validity,n,n);
	if(err)
		goto d_m;

	for(i = 0; i < orders; i++)
	{
		// posets is used for permutation computations
		// it should contain vertices
		err = igraph_vector_init_seq(&(posets[i]),0,n-1);
		if(err)
			goto d_p;

		err = igraph_vector_init(&(index[i]),n);
		if(err)
			goto d_i;
	}

	// zero all structs
	igraph_matrix_null(&m);
	igraph_matrix_null(&edge_validity);

	// use gsl to shuffle each poset
	for( j = 0; j < orders; j++)
		gsl_ran_shuffle(r,VECTOR(posets[j]), n, sizeof(VECTOR(posets[j])[0]));

	// index saves the indices of each vertex in each permutation
	for( i = 0; i < orders; i++)
		for( j = 0; j < n; j++)
		{
			k = VECTOR(posets[i])[j];
			VECTOR(index[i])[k] = j;
		}

	// edge_validity count if an edge is in all permutations
	for( i = 0; i < n; i++)
		for( j = 0; j < n; j++)
			for( k = 0; k < orders; k++)
				if( VECTOR(index[k])[i] < VECTOR(index[k])[j])
					igraph_matrix_set(&edge_validity,i,j,
						igraph_matrix_e(&edge_validity,i,j)+1);

	// if an edge is present in all permutations then add it to the graph
	for( i = 0; i < n; i++)
		for( j = 0; j < n; j++)
			if(igraph_matrix_e(&edge_validity,i,j) == orders)
				igraph_matrix_set(&m,i,j,1);

	// translate the matrix to a graph
	err = igraph_adjacency(g,&m,IGRAPH_ADJ_DIRECTED);

	// cleanup
	// we destroy vector in the opposite way we initialized them
	// and skip the index one if any failed
	i = orders -1;
	for(; i >= 0; i--)
	{
		igraph_vector_destroy(&(index[i]));
d_i:
		igraph_vector_destroy(&(posets[i]));
d_p:
		;
	}

	igraph_matrix_destroy(&edge_validity);
d_m:
	igraph_matrix_destroy(&m);
	if(err)
	{
f_g:
		free(g);
		g = NULL;
	}
	return g;
}
