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
#define GGEN_CGRAPH_DEFAULT_VALUE ""
#include <graphviz/cgraph.h>
#include <stdio.h>
#include <string.h>

#include "ggen.h"
#include "error.h"

/* This use cgraph to read/write graphviz DOT format
 * This includes some amount of default values for buffers and attribute names.
 */

char *ggen_vname(igraph_t *g, char *buf, unsigned long id)
{
	char *str = NULL;
	igraph_bool_t has;
	/* check if the attribute exists */
	has = igraph_cattribute_has_attr(g,IGRAPH_ATTRIBUTE_VERTEX,
					GGEN_VERTEX_NAME_ATTR);
	if(has)
	{
		str = (char*)VAS(g,GGEN_VERTEX_NAME_ATTR,id);
		return str;
	}
	else
	{
		/* defaults to writing the vertex id in a string */
		snprintf(buf,GGEN_DEFAULT_NAME_SIZE,"%lu",id);
		return NULL;
	}
}

/* find an id in an array */
static int find_id(unsigned long *ret, unsigned long id, igraph_vector_t v, 
		unsigned long size)
{
	unsigned long i;
	for(i = 0; i < size; i++)
		if(VECTOR(v)[i] == id)
		{
			*ret = i;
			return GGEN_SUCCESS;
		}

	return GGEN_FAILURE;
}


int ggen_read_graph(igraph_t *g, FILE *input)
{
	Agraph_t *cg;
	Agnode_t *v;
	Agedge_t *e;
	igraph_vector_t edges;
	igraph_vector_t vertices;
	int err;
	unsigned long esize;
	unsigned long vsize;
	unsigned long from, to;
	igraph_integer_t eid;
	Agsym_t *att;

	ggen_error_start_stack();
	if(g == NULL)
		GGEN_SET_ERRNO(GGEN_EINVAL);

	/* read the graph */
	cg = agread((void *)input,NULL);
	if(!cg)
		GGEN_SET_ERRNO(GGEN_CGRAPH_ERROR);
	GGEN_FINALLY(agclose,cg);

	if(!agisdirected(cg))
		GGEN_SET_ERRNO(GGEN_ENODAG);

	GGEN_CHECK_IGRAPH(igraph_vector_init(&edges,2*agnedges(cg)));
	GGEN_FINALLY(igraph_vector_destroy,&edges);

	GGEN_CHECK_IGRAPH(igraph_vector_init(&vertices,agnnodes(cg)));
	GGEN_FINALLY(igraph_vector_destroy,&vertices);

	/* init igraph */
	igraph_empty(g,agnnodes(cg),1);

	/* asign id to each vertex */
	vsize = 0;
	for(v = agfstnode(cg); v; v = agnxtnode(cg,v))
		VECTOR(vertices)[vsize++] = AGID(v);

	/* loop through each edge */
	esize = 0;
	for(v = agfstnode(cg); v; v = agnxtnode(cg,v))
	{
		GGEN_CHECK_INTERNAL(find_id(&from,AGID(v),vertices,vsize));
		for(e = agfstout(cg,v); e; e = agnxtout(cg,e))
		{
			GGEN_CHECK_INTERNAL(find_id(&to,AGID(aghead(e)),vertices,vsize));
			VECTOR(edges)[esize++] = from;
			VECTOR(edges)[esize++] = to;
		}
	}

	/* finish the igraph */
	GGEN_CHECK_IGRAPH(igraph_add_edges(g,&edges,NULL));

	/* read graph properties */
	att = agnxtattr(cg,AGRAPH,NULL);
	while(att != NULL)
	{
		/* copy this attribute to igraph */
		SETGAS(g,att->name,agxget(cg,att));
		att = agnxtattr(cg,AGRAPH,att);
	}
	/* we keep the graph name using a special attribute */
	SETGAS(g,GGEN_GRAPH_NAME_ATTR,agnameof(cg));

	/* read vertex properties */
	att = agnxtattr(cg,AGNODE,NULL);
	while(att != NULL)
	{
		/* iterate over all vertices for this attribute */
		for(v = agfstnode(cg); v; v = agnxtnode(cg,v))
		{
			GGEN_CHECK_INTERNAL(find_id(&from,AGID(v),vertices,vsize));
			SETVAS(g,att->name,from,agxget(v,att));
		}
		att = agnxtattr(cg,AGNODE,att);
	}
	/* we keep each vertex name in a special attribute */
	for(v = agfstnode(cg); v; v = agnxtnode(cg,v))
	{
			GGEN_CHECK_INTERNAL(find_id(&from,AGID(v),vertices,vsize));
			SETVAS(g,GGEN_VERTEX_NAME_ATTR,from,agnameof(v));
	}


	/* read edges properties */
	att = agnxtattr(cg,AGEDGE,NULL);
	while(att != NULL)
	{
		/* the only way to iterate over all edges is to iterate
		 * over the vertices */
		for(v = agfstnode(cg); v; v = agnxtnode(cg,v))
		{
			GGEN_CHECK_INTERNAL(find_id(&from,AGID(v),vertices,vsize));
			for(e = agfstout(cg,v); e; e = agnxtout(cg,e))
			{
				GGEN_CHECK_INTERNAL(find_id(&to,AGID(aghead(e)),vertices,vsize));
				igraph_get_eid(g,&eid,from,to,1,0);
				SETEAS(g,att->name,eid,agxget(e,att));
			}
		}
		att = agnxtattr(cg,AGEDGE,att);
	}
	
	ggen_error_clean(1);
	return GGEN_SUCCESS;
ggen_error_label:
	return GGEN_FAILURE;
}

int ggen_write_graph(igraph_t *g, FILE *output)
{
	Agraph_t *cg;
	Agnode_t *f,*t;
	Agedge_t *edge;
	igraph_vector_ptr_t vertices;
	igraph_eit_t eit;
	int err;
	unsigned long i,j;
	unsigned long vcount;
	igraph_integer_t from,to;
	char name[GGEN_DEFAULT_NAME_SIZE];
	char *str = NULL;
	igraph_strvector_t gnames,vnames,enames;
	igraph_vector_t gtypes,vtypes,etypes;
	Agsym_t *attr;
	igraph_bool_t hasattr;

	ggen_error_start_stack();
	if(g == NULL)
		GGEN_SET_ERRNO(GGEN_EINVAL);
	vcount = igraph_vcount(g);

	GGEN_CHECK_IGRAPH(igraph_vector_ptr_init(&vertices,vcount));
	GGEN_FINALLY(igraph_vector_ptr_destroy,&vertices);

	/* open graph
	 * its name is saved in __ggen_graph_name if it exists
	 */
	hasattr = igraph_cattribute_has_attr(g, IGRAPH_ATTRIBUTE_GRAPH,
			GGEN_GRAPH_NAME_ATTR);
	if(hasattr)
	{
		str =(char *) GAS(g,GGEN_GRAPH_NAME_ATTR);
		cg = agopen(str,Agdirected,NULL);
	}
	else
		cg = agopen(GGEN_DEFAULT_GRAPH_NAME,Agdirected,NULL);

	if(!cg)
		GGEN_SET_ERRNO(GGEN_CGRAPH_ERROR);
	GGEN_FINALLY(agclose,cg);

	/* save a pointer to each vertex */
	for(i = 0; i < vcount; i++)
	{
		/* find a vertex name */
		str = ggen_vname(g,name,i);
		if(!str)
			f = agnode(cg,name,1);
		else
			f = agnode(cg,str,1);
		VECTOR(vertices)[i] = (void *)f;
	}

	/* now loop through edges in the igraph */
	GGEN_CHECK_IGRAPH(igraph_eit_create(g,igraph_ess_all(IGRAPH_EDGEORDER_ID),&eit));
	GGEN_FINALLY(igraph_eit_destroy,&eit);

	for(IGRAPH_EIT_RESET(eit); !IGRAPH_EIT_END(eit); IGRAPH_EIT_NEXT(eit))
	{
		igraph_edge(g,IGRAPH_EIT_GET(eit),&from,&to);
		f = (Agnode_t *) VECTOR(vertices)[(unsigned long)from];
		t = (Agnode_t *) VECTOR(vertices)[(unsigned long)to];
		agedge(cg,f,t,NULL,1);
	}

	/* find all properties */
	GGEN_CHECK_IGRAPH(igraph_strvector_init(&gnames,1));
	GGEN_FINALLY(igraph_strvector_destroy,&gnames);
	GGEN_CHECK_IGRAPH(igraph_strvector_init(&vnames,vcount));
	GGEN_FINALLY(igraph_strvector_destroy,&vnames);
	GGEN_CHECK_IGRAPH(igraph_strvector_init(&enames,igraph_ecount(g)));
	GGEN_FINALLY(igraph_strvector_destroy,&enames);
	GGEN_CHECK_IGRAPH(igraph_vector_init(&gtypes,1));
	GGEN_FINALLY(igraph_vector_destroy,&gtypes);
	GGEN_CHECK_IGRAPH(igraph_vector_init(&vtypes,vcount));
	GGEN_FINALLY(igraph_vector_destroy,&vtypes);
	GGEN_CHECK_IGRAPH(igraph_vector_init(&etypes,igraph_ecount(g)));
	GGEN_FINALLY(igraph_vector_destroy,&etypes);

	GGEN_CHECK_IGRAPH(igraph_cattribute_list(g,&gnames,&gtypes,&vnames,&vtypes,&enames,&etypes));

	/* add graph properties */
	for(i = 0; i < igraph_strvector_size(&gnames); i++)
	{
		if(strcmp(GGEN_GRAPH_NAME_ATTR,STR(gnames,i)))
		{
			if(VECTOR(gtypes)[i]==IGRAPH_ATTRIBUTE_NUMERIC) {
				snprintf(name,GGEN_DEFAULT_NAME_SIZE,"%g",
						(double)GAN(g,STR(gnames,i)));
				agattr(cg,AGRAPH,(char *)STR(gnames,i),name);
			}
			else
				agattr(cg,AGRAPH,(char *)STR(gnames,i),
						(char *)GAS(g,STR(gnames,i)));
		}
	}

	/* add vertex properties */
	for(i = 0; i < igraph_strvector_size(&vnames); i++)
	{
		if(strcmp(GGEN_VERTEX_NAME_ATTR,STR(vnames,i)))
		{
			/* creates the attribute but we still need to set it for each vertex */
			attr = agattr(cg,AGNODE,(char *)STR(vnames,i),GGEN_CGRAPH_DEFAULT_VALUE);
			for(j = 0; j < vcount; j++)
			{
				f = (Agnode_t *) VECTOR(vertices)[j];
				if(VECTOR(vtypes)[i]==IGRAPH_ATTRIBUTE_NUMERIC) {
					snprintf(name,GGEN_DEFAULT_NAME_SIZE,"%g",
							(double)VAN(g,STR(vnames,i),j));
					agxset(f,attr,name);
				}
				else
					agxset(f,attr,(char *)VAS(g,STR(vnames,i),j));
			}
		}
	}

	/* add edges properties */
	for(i = 0; i < igraph_strvector_size(&enames); i++)
	{
		/* creates the attribute but we still need to set it for each edge */
		attr = agattr(cg,AGEDGE,(char *)STR(enames,i),GGEN_CGRAPH_DEFAULT_VALUE);
		for(j = 0; j < igraph_ecount(g); j++)
		{
			igraph_edge(g,j,&from,&to);
			f = (Agnode_t *) VECTOR(vertices)[(unsigned long)from];
			t = (Agnode_t *) VECTOR(vertices)[(unsigned long)to];
			edge = agedge(cg,f,t,NULL,0);
			if(VECTOR(etypes)[i]==IGRAPH_ATTRIBUTE_NUMERIC) {
				snprintf(name,GGEN_DEFAULT_NAME_SIZE,"%g",
						(double)EAN(g,STR(enames,i),j));
				agxset(edge,attr,name);
			}
			else
				agxset(edge,attr,(char *)EAS(g,STR(enames,i),j));
		}
	}

	/* write the graph */
	if(agwrite(cg,(void *)output))
		GGEN_SET_ERRNO(GGEN_CGRAPH_ERROR);

	ggen_error_clean(1);
	return GGEN_SUCCESS;
ggen_error_label:
	return GGEN_FAILURE;
}
