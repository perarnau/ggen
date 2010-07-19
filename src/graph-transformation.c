/* Copyright Swann Perarnau 2009
 *
 *   contributor(s) :
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
/* GGen is a random graph generator :
 * it provides means to generate a graph following a
 * collection of methods found in the litterature.
 *
 * This is a research project founded by the MOAIS Team,
 * INRIA, Grenoble Universities.
 */

#include "ggen.h"

int ggen_transform_add(igraph_t *g, enum ggen_transform_t t)
{
	igraph_vector_t vertices;
	igraph_vector_t degrees;
	igraph_vector_t edges;
	unsigned int i,vcount,ssize;
	int err;

	if(g == NULL)
		return 1;

	vcount = igraph_vcount(g);

	err = igraph_vector_init(&vertices,vcount);
	if(err) return 1;

	err = igraph_vector_init(&degrees,vcount);
	if(err) goto error_id;

	/* find degree of each vertex, in if new source is wanted. */
	err = igraph_degree(g,&degrees,igraph_vss_all(),
		(t==GGEN_TRANSFORM_SOURCE)?IGRAPH_IN:IGRAPH_OUT,0);
	if(err) goto error;

	/* only sources or sinks are of interest */
	ssize = 0;
	for(i = 0; i < vcount; i++)
		if(VECTOR(degrees)[i] == 0)
			VECTOR(vertices)[ssize++] = i;

	/* we have something to do */
	if(ssize > 0)
	{
		err = igraph_add_vertices(g,1,NULL);
		if(err) goto error;

		err = igraph_vector_init(&edges,ssize*2);
		if(err) goto error;

		for(i = 0; i < ssize; i++)
		{
			if(t == GGEN_TRANSFORM_SOURCE)
			{
				VECTOR(edges)[2*i] = vcount;
				VECTOR(edges)[2*i+1] = VECTOR(vertices)[i];
			}
			else
			{
				VECTOR(edges)[2*i] = VECTOR(vertices)[i];
				VECTOR(edges)[2*i+1] = vcount;
			}
		}

		err = igraph_add_edges(g,&edges,NULL);
		igraph_vector_destroy(&edges);
		if(err) goto error;
	}
error:
	igraph_vector_destroy(&degrees);
error_id:
	igraph_vector_destroy(&vertices);
	return err;
}

int ggen_transform_delete(igraph_t *g, enum ggen_transform_t t)
{
	igraph_vector_t vertices;
	igraph_vector_t degrees;
	unsigned int i,vcount,ssize;
	int err;

	if(g == NULL)
		return 1;

	vcount = igraph_vcount(g);

	err = igraph_vector_init(&vertices,vcount);
	if(err) return 1;

	err = igraph_vector_init(&degrees,vcount);
	if(err) goto error_id;

	/* find degree of each vertex, igraph_in if we need to identify sources. */
	err = igraph_degree(g,&degrees,igraph_vss_all(),
		(t==GGEN_TRANSFORM_SOURCE)?IGRAPH_IN:IGRAPH_OUT,0);
	if(err) goto error;

	/* only sources or sinks are of interest */
	ssize = 0;
	for(i = 0; i < vcount; i++)
		if(VECTOR(degrees)[i] == 0)
			VECTOR(vertices)[ssize++] = i;

	/* delete all identified vertices */
	if(ssize > 0)
	{
		/* we should resize the array to avoid strange behaviors */
		err = igraph_vector_resize(&vertices,ssize);
		if(err) goto error;

		err = igraph_delete_vertices(g,igraph_vss_vector(&vertices));
		if(err) goto error;
	}
error:
	igraph_vector_destroy(&degrees);
error_id:
	igraph_vector_destroy(&vertices);
	return err;
}
