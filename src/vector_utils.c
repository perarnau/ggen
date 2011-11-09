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

#include "vector_utils.h"

int vector_uniq_sorted(igraph_vector_t *v)
{
	unsigned long i,pos,l;
	/* basic idea : remember the last good number,
	 * test each index against it.
	 * It is that simple because the vector is sorted
	 */
	pos = 0;
	l = 1;
	for(i = 1; i < igraph_vector_size(v); i++)
	{
		if(VECTOR(*v)[i] != VECTOR(*v)[pos])
		{
			pos++;
			VECTOR(*v)[pos] = VECTOR(*v)[i];
		}
	}
	igraph_vector_resize(v,pos+1);
	return 0;
}

int vector_uniq(igraph_vector_t *v)
{
	igraph_vector_sort(v);
	return vector_uniq_sorted(v);
}

int vector_diff(igraph_vector_t *to, igraph_vector_t *from)
{
	unsigned long i,j,found,ts,t,f;
	ts = igraph_vector_size(to);
	for(i = 0; i < igraph_vector_size(from);i++)
	{
		for(j = 0; j < ts; j++)
		{
			t = (unsigned long)VECTOR(*to)[j];
			f = (unsigned long)VECTOR(*from)[i];
			if(t == f)
			{
				ts--;
				igraph_vector_remove(to,j);
				j--;
			}
		}
	}
	return 0;
}

int vector_union(igraph_vector_t *to, igraph_vector_t *from)
{
	igraph_vector_append(to,from);
	return vector_uniq(to);
}
