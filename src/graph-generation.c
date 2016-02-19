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
#include "error.h"

/* Erdos-Renyi : G(n,M)
*/
igraph_t *ggen_generate_erdos_gnm(gsl_rng *r, unsigned long n, unsigned long m)
{
	igraph_matrix_t adj;
	igraph_t *g = NULL;
	int err;
	unsigned long i,j;
	unsigned long added_edges = 0;

	ggen_error_start_stack();
	if(r == NULL)
		GGEN_SET_ERRNO(GGEN_EINVAL);

	if(m > (n*(n-1)/2))
		GGEN_SET_ERRNO(GGEN_EINVAL);

	g = malloc(sizeof(igraph_t));
	GGEN_CHECK_ALLOC(g);
	GGEN_FINALLY3(free,g,1);

	if(m == 0 || n <= 1)
	{
		GGEN_CHECK_IGRAPH(igraph_empty(g,n,1));
		goto end;
	}
	if(m == (n*(n-1))/2)
	{
		GGEN_CHECK_IGRAPH(igraph_full_citation(g,n,1));
		goto end;
	}

	GGEN_CHECK_IGRAPH(igraph_matrix_init(&adj,n,n));
	GGEN_FINALLY(igraph_matrix_destroy,&adj);

	igraph_matrix_null(&adj);

	while(added_edges < m) {
		GGEN_CHECK_GSL_DO(i = gsl_rng_uniform_int(r,n));
		GGEN_CHECK_GSL_DO(j = gsl_rng_uniform_int(r,n));

                if(i < j && igraph_matrix_e(&adj,i,j) == 0)
                {
			igraph_matrix_set(&adj,i,j,1);
			added_edges++;
                }

        }

	GGEN_CHECK_IGRAPH(igraph_adjacency(g,&adj,IGRAPH_ADJ_DIRECTED));
end:
	ggen_error_clean(1);
	return g;
ggen_error_label:
	return NULL;
}

/* Erdos-Renyi : G(n,p)
*/
igraph_t *ggen_generate_erdos_gnp(gsl_rng *r, unsigned long n, double p)
{
	igraph_matrix_t m;
	igraph_t *g = NULL;
	int err;
	unsigned long i,j;

	ggen_error_start_stack();
	if(r == NULL)
		GGEN_SET_ERRNO(GGEN_EINVAL);

	if(p < 0.0 || p > 1.0)
		GGEN_SET_ERRNO(GGEN_EINVAL);

	g = malloc(sizeof(igraph_t));
	GGEN_CHECK_ALLOC(g);
	GGEN_FINALLY3(free,g,1);

	if(p == 0.0)
	{
		GGEN_CHECK_IGRAPH(igraph_empty(g,n,1));
		goto end;
	}
	if(p == 1.0)
	{
		GGEN_CHECK_IGRAPH(igraph_full_citation(g,n,1));
		goto end;
	}

	GGEN_CHECK_IGRAPH(igraph_matrix_init(&m,n,n));
	GGEN_FINALLY(igraph_matrix_destroy,&m);

	for(i = 0; i < n; i++)
		for(j = 0; j < n; j++)
			if(i < j)
				// coin flipping to determine if we add an edge or not
				igraph_matrix_set(&m,i,j,gsl_ran_bernoulli(r,p));
			else
				igraph_matrix_set(&m,i,j,0);

	GGEN_CHECK_IGRAPH(igraph_adjacency(g,&m,IGRAPH_ADJ_DIRECTED));
end:
	ggen_error_clean(1);
	return g;
ggen_error_label:
	return NULL;
}

igraph_t *ggen_generate_erdos_lbl(gsl_rng *r, unsigned long n, double p, unsigned long nbl)
{
	igraph_t *g = NULL;
	igraph_matrix_t m;
	igraph_vector_t layers;
	unsigned long i,j;
	int err;

	ggen_error_start_stack();
	if(r == NULL)
		GGEN_SET_ERRNO(GGEN_EINVAL);

	if(p < 0.0 || p > 1.0)
		GGEN_SET_ERRNO(GGEN_EINVAL);

	if(nbl > n || nbl == 0)
		GGEN_SET_ERRNO(GGEN_EINVAL);

	g = malloc(sizeof(igraph_t));
	GGEN_CHECK_ALLOC(g);
	GGEN_FINALLY3(free,g,1);

	if(p == 0.0)
	{
		GGEN_CHECK_IGRAPH(igraph_empty(g,n,1));
		goto end;
	}
	if(p == 1.0 && nbl == n)
	{
		GGEN_CHECK_IGRAPH(igraph_full_citation(g,n,1));
		goto end;
	}

	GGEN_CHECK_IGRAPH(igraph_matrix_init(&m,n,n));
	GGEN_FINALLY(igraph_matrix_destroy,&m);

	GGEN_CHECK_IGRAPH(igraph_vector_init(&layers,n));
	GGEN_FINALLY(igraph_vector_destroy,&layers);

	// asign to each vertex a layer
	for(i = 0; i < n; i++)
	{
		GGEN_CHECK_GSL_DO(j = gsl_rng_uniform_int(r,nbl));
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
	GGEN_CHECK_IGRAPH(igraph_adjacency(g,&m,IGRAPH_ADJ_DIRECTED));
end:
	ggen_error_clean(1);
	return g;
ggen_error_label:
	return NULL;
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

	ggen_error_start_stack();
	if(r == NULL)
		GGEN_SET_ERRNO(GGEN_EINVAL);

	if(id == 0 || od == 0 || od > n || id > n)
		GGEN_SET_ERRNO(GGEN_EINVAL);

	g = malloc(sizeof(igraph_t));
	GGEN_CHECK_ALLOC(g);
	GGEN_FINALLY3(free,g,1);

	GGEN_CHECK_IGRAPH(igraph_empty(g,1,1));
	GGEN_FINALLY3(igraph_destroy,g,1);

	GGEN_CHECK_IGRAPH(igraph_vector_init(&available_od,n));
	GGEN_FINALLY(igraph_vector_destroy,&available_od);
	GGEN_CHECK_IGRAPH(igraph_vector_init(&out_degrees,n));
	GGEN_FINALLY(igraph_vector_destroy,&out_degrees);
	GGEN_CHECK_IGRAPH(igraph_vector_init(&vertices,n));
	GGEN_FINALLY(igraph_vector_destroy,&vertices);
	GGEN_CHECK_IGRAPH(igraph_vector_init(&choice,n));
	GGEN_FINALLY(igraph_vector_destroy,&choice);
	GGEN_CHECK_IGRAPH(igraph_vector_init(&edges,n*2));
	GGEN_FINALLY(igraph_vector_destroy,&edges);
	while(vcount < n)
	{
		// never trigger errors as it doesn't allocate or free memory
		igraph_vector_resize(&available_od,vcount);
		igraph_vector_resize(&out_degrees,vcount);
		igraph_vector_resize(&vertices,vcount);

		// compute the available out degree of each vertex
		GGEN_CHECK_IGRAPH(igraph_degree(g,&out_degrees,igraph_vss_all(),IGRAPH_OUT,0));

		// fill available with od and substract out_degrees
		igraph_vector_fill(&available_od,od);
		GGEN_CHECK_IGRAPH(igraph_vector_sub(&available_od,&out_degrees));

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
			GGEN_CHECK_GSL_DO(i = gsl_rng_uniform_int(r,j));

			// how many children ?
			GGEN_CHECK_GSL_DO(j = gsl_rng_uniform_int(r,max));
			j = j+1;

			// create all new nodes and add edges
			GGEN_CHECK_IGRAPH(igraph_add_vertices(g,j,NULL));

			// cannot fail
			igraph_vector_resize(&edges,j*2);

			for(k = 0; k < j; k++)
			{
				VECTOR(edges)[2*k] = VECTOR(vertices)[i];
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
			GGEN_CHECK_GSL_DO(k = gsl_rng_uniform_int(r,max));
			k = k+1;

			// choose that many nodes and add edges from them to the new node
			// cannot fail either
			igraph_vector_resize(&choice,k);

			gsl_ran_choose(r,VECTOR(choice),k,
				VECTOR(vertices),j,sizeof(VECTOR(vertices)[0]));

			// add a vertex to the graph
			GGEN_CHECK_IGRAPH(igraph_add_vertices(g,1,NULL));

			igraph_vector_resize(&edges,k*2);
			// be carefull, vcount is the last ID of vertices not vcount +1
			for(i = 0; i < k; i++)
			{
				VECTOR(edges)[2*i] = VECTOR(choice)[i];
				VECTOR(edges)[2*i+1] = vcount;
			}
			vcount++;

		}
		// in all cases, edges should be added
		GGEN_CHECK_IGRAPH(igraph_add_edges(g,&edges,NULL));
	}
	ggen_error_clean(1);
	return g;
ggen_error_label:
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
	igraph_vector_ptr_t posets;
	igraph_vector_ptr_t indexes;
	igraph_vector_t *v,*w;

	ggen_error_start_stack();
	if(r == NULL)
		GGEN_SET_ERRNO(GGEN_EINVAL);

	if(orders == 0)
		GGEN_SET_ERRNO(GGEN_EINVAL);

	// init structures
	g = malloc(sizeof(igraph_t));
	GGEN_CHECK_ALLOC(g);
	GGEN_FINALLY3(free,g,1);

	GGEN_CHECK_IGRAPH(igraph_matrix_init(&m,n,n));
	GGEN_FINALLY(igraph_matrix_destroy,&m);

	GGEN_CHECK_IGRAPH(igraph_matrix_init(&edge_validity,n,n));
	GGEN_FINALLY(igraph_matrix_destroy,&edge_validity);

	GGEN_CHECK_IGRAPH(igraph_vector_ptr_init(&posets,orders));
	IGRAPH_VECTOR_PTR_SET_ITEM_DESTRUCTOR(&posets,igraph_vector_destroy);
	GGEN_FINALLY(igraph_vector_ptr_destroy_all,&posets);

	GGEN_CHECK_IGRAPH(igraph_vector_ptr_init(&indexes,orders));
	IGRAPH_VECTOR_PTR_SET_ITEM_DESTRUCTOR(&indexes,igraph_vector_destroy);
	GGEN_FINALLY(igraph_vector_ptr_destroy_all,&indexes);

	for(i = 0; i < orders; i++)
	{
		// posets is used for permutation computations
		// it should contain vertices
		VECTOR(posets)[i] = calloc(1,sizeof(igraph_vector_t));
		GGEN_CHECK_ALLOC(VECTOR(posets)[i]);
		GGEN_CHECK_IGRAPH_VECTPTR(igraph_vector_init_seq(VECTOR(posets)[i],0,n-1),posets,i);

		VECTOR(indexes)[i] = calloc(1,sizeof(igraph_vector_t));
		GGEN_CHECK_ALLOC(VECTOR(indexes)[i]);
		GGEN_CHECK_IGRAPH_VECTPTR(igraph_vector_init(VECTOR(indexes)[i],n),indexes,i);
	}

	// zero all structs
	igraph_matrix_null(&m);
	igraph_matrix_null(&edge_validity);

	// use gsl to shuffle each poset
	for( j = 0; j < orders; j++)
	{
		v = VECTOR(posets)[j];
		GGEN_CHECK_GSL_DO(gsl_ran_shuffle(r,VECTOR(*v), n, sizeof(VECTOR(*v)[0])));
	}
	// index saves the indices of each vertex in each permutation
	for( i = 0; i < orders; i++)
		for( j = 0; j < n; j++)
		{
			v = VECTOR(posets)[i];
			w = VECTOR(indexes)[i];
			k = VECTOR(*v)[j];
			VECTOR(*w)[k] = j;
		}

	// edge_validity count if an edge is in all permutations
	for( i = 0; i < n; i++)
		for( j = 0; j < n; j++)
			for( k = 0; k < orders; k++)
			{
				v = VECTOR(indexes)[k];
				if( VECTOR(*v)[i] < VECTOR(*v)[j])
					igraph_matrix_set(&edge_validity,i,j,
						igraph_matrix_e(&edge_validity,i,j)+1);
			}

	// if an edge is present in all permutations then add it to the graph
	for( i = 0; i < n; i++)
		for( j = 0; j < n; j++)
			if(igraph_matrix_e(&edge_validity,i,j) == orders)
				igraph_matrix_set(&m,i,j,1);

	// translate the matrix to a graph
	GGEN_CHECK_IGRAPH(igraph_adjacency(g,&m,IGRAPH_ADJ_DIRECTED));

	ggen_error_clean(1);
	return g;
ggen_error_label:
	return NULL;
}
