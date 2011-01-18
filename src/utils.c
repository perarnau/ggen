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

#include "utils.h"
#include "log.h"

/* gsl rng stuff */
int ggen_rng_init(gsl_rng **r)
{
	const gsl_rng_type *T;
	gsl_rng_env_setup();

	T = gsl_rng_default;
	*r = gsl_rng_alloc(T);

	info("Using %s as RNG.\n",gsl_rng_name(*r));
	info("Using %lu as RNG seed.\n",gsl_rng_default_seed);
	return 0;
}

int ggen_rng_save(gsl_rng **r,const char *file)
{
	FILE *f;
	int err;
	f = fopen(file,"w");
	if(!f) return 1;

	err = gsl_rng_fwrite(f,*r);

	fclose(f);
	return err;
}

int ggen_rng_load(gsl_rng **r,const char *file)
{
	FILE *f;
	int err;
	f = fopen(file,"r");
	if(!f) return 1;

	err = gsl_rng_fread(f,*r);

	fclose(f);
	return err;
}

/* string conversion */
int s2ul(char *s,unsigned long *l)
{
	unsigned long r;
	char *err;
	/* strtoul manual recommends error checking
	 * using errno */
	errno = 0;
	r = strtoul(s,&err,0);
	if(errno)
	{
		error("Failed to convert string to unsigned long: %s\n",strerror(errno));
		return 1;
	}
	*l = r;
	return 0;
}

int s2d(char *s,double *d)
{
	double r;
	char *err;
	/* strtod manual recommends error checking
	 * using errno */
	errno = 0;
	r = strtod(s,&err);
	if(errno)
	{
		error("Failed to convert string to double: %s\n",strerror(errno));
		return 1;
	}
	*d = r;
	return 0;

}

/* graph io */
/* this use cgraph to read/write dot */

/* find an id in an array */
static unsigned long find_id(unsigned long id,igraph_vector_t v,unsigned long n)
{
	unsigned long i;
	for(i = 0; i < n; i++)
		if(VECTOR(v)[i] == id)
			return i;

	error("BUG: this function should not fail: %s\n",__func__);
	return 0;
}


int ggen_read_graph(igraph_t *g,FILE *input)
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

	/* read the graph */
	cg = agread((void *)input,NULL);
	if(!cg) return 1;

	if(!agisdirected(cg))
	{
		error("Input graph is undirected\n");
		err = 1;
		goto error_d;
	}

	/* init edge array */
	err = igraph_vector_init(&edges,2*agnedges(cg));
	if(err) goto error_d;

	err = igraph_vector_init(&vertices,agnnodes(cg));
	if(err) goto error_de;

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
		from = find_id(AGID(v),vertices,vsize);
		for(e = agfstout(cg,v); e; e = agnxtout(cg,e))
		{
			to = find_id(AGID(aghead(e)),vertices,vsize);
			VECTOR(edges)[esize++] = from;
			VECTOR(edges)[esize++] = to;
		}
	}

	/* finish the igraph */
	err = igraph_add_edges(g,&edges,NULL);
	if(err) goto error;

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
			from = find_id(AGID(v),vertices,vsize);
			SETVAS(g,att->name,from,agxget(v,att));
		}
		att = agnxtattr(cg,AGNODE,att);
	}
	/* we keep each vertex name in a special attribute */
	for(v = agfstnode(cg); v; v = agnxtnode(cg,v))
	{
			from = find_id(AGID(v),vertices,vsize);
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
			from = find_id(AGID(v),vertices,vsize);
			for(e = agfstout(cg,v); e; e = agnxtout(cg,e))
			{
				to = find_id(AGID(aghead(e)),vertices,vsize);
				igraph_get_eid(g,&eid,from,to,1);
				SETEAS(g,att->name,eid,agxget(e,att));
			}
		}
		att = agnxtattr(cg,AGEDGE,att);
	}

	goto cleanup;
error:
	igraph_destroy(g);
cleanup:
	igraph_vector_destroy(&vertices);
error_de:
	igraph_vector_destroy(&edges);
error_d:
	agclose(cg);
	return err;
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
	unsigned long vcount = igraph_vcount(g);
	igraph_integer_t from,to;
	char name[GGEN_DEFAULT_NAME_SIZE];
	char *str = NULL;
	igraph_strvector_t gnames,vnames,enames;
	igraph_vector_t gtypes,vtypes,etypes;
	Agsym_t *attr;
	/* see warning below */
	igraph_error_handler_t *error_handler;

	err = igraph_vector_ptr_init(&vertices,vcount);
	if(err) return 1;

	/* WARNING: this should be changed if igraph-0.6 gets
	 * stable.
	 * We need to ignore some igraph_cattribute errors
	 * because we try to retrieve special attributes (ggen specifics).
	 * igraph version 0.6 include a cattribute_has_attr that should be
	 * used instead of ignoring errors.
	 */
	error_handler = igraph_set_error_handler(igraph_error_handler_ignore);

	/* open graph
	 * its name is saved in __ggen_graph_name if it exists
	 */
	str =(char *) GAS(g,GGEN_GRAPH_NAME_ATTR);
	if(!str)
		cg = agopen(GGEN_DEFAULT_GRAPH_NAME,Agdirected,NULL);
	else
		cg = agopen(str,Agdirected,NULL);

	if(!cg)
	{
		err = 1;
		goto d_v;
	}

	/* save a pointer to each vertex */
	for(i = 0; i < vcount; i++)
	{
		/* try to find a vertex name */
		str = (char *)VAS(g,GGEN_VERTEX_NAME_ATTR,i);
		if(!str)
		{
			snprintf(name,GGEN_DEFAULT_NAME_SIZE,"%lu",i);
			f = agnode(cg,name,1);
		}
		else
			f = agnode(cg,str,1);
		VECTOR(vertices)[i] = (void *)f;
	}

	/* We have finished with dangerous attributes accesses */
	igraph_set_error_handler(error_handler);

	/* now loop through edges in the igraph */
	err = igraph_eit_create(g,igraph_ess_all(IGRAPH_EDGEORDER_ID),&eit);
	if(err) goto c_ag;

	for(IGRAPH_EIT_RESET(eit); !IGRAPH_EIT_END(eit); IGRAPH_EIT_NEXT(eit))
	{
		err = igraph_edge(g,IGRAPH_EIT_GET(eit),&from,&to);
		if(err) goto d_eit;

		f = (Agnode_t *) VECTOR(vertices)[(unsigned long)from];
		t = (Agnode_t *) VECTOR(vertices)[(unsigned long)to];
		agedge(cg,f,t,NULL,1);
	}

	/* find all properties */
	igraph_strvector_init(&gnames,1);
	igraph_strvector_init(&vnames,vcount);
	igraph_strvector_init(&enames,igraph_ecount(g));
	igraph_vector_init(&gtypes,1);
	igraph_vector_init(&vtypes,vcount);
	igraph_vector_init(&etypes,igraph_ecount(g));

	err = igraph_cattribute_list(g,&gnames,&gtypes,&vnames,&vtypes,&enames,&etypes);
	if(err) goto d_eit;

	/* add graph properties */
	for(i = 0; i < igraph_strvector_size(&gnames); i++)
	{
		if(VECTOR(gtypes)[i]==IGRAPH_ATTRIBUTE_NUMERIC) {
			snprintf(name,GGEN_DEFAULT_NAME_SIZE,"%f",(double)GAN(g,STR(gnames,i)));
			agattr(cg,AGRAPH,(char *)STR(gnames,i),name);
		}
		else
			agattr(cg,AGRAPH,(char *)STR(gnames,i),(char *)GAS(g,STR(gnames,i)));
	}

	/* add vertex properties */
	for(i = 0; i < igraph_strvector_size(&vnames); i++)
	{
		/* creates the attribute but we still need to set it for each vertex */
		attr = agattr(cg,AGNODE,(char *)STR(vnames,i),GGEN_CGRAPH_DEFAULT_VALUE);
		for(j = 0; j < vcount; j++)
		{
			f = (Agnode_t *) VECTOR(vertices)[j];
			if(VECTOR(vtypes)[i]==IGRAPH_ATTRIBUTE_NUMERIC) {
				snprintf(name,GGEN_DEFAULT_NAME_SIZE,"%f",
						(double)VAN(g,STR(vnames,i),j));
				agxset(f,attr,name);
			}
			else
				agxset(f,attr,(char *)VAS(g,STR(vnames,i),j));
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
				snprintf(name,GGEN_DEFAULT_NAME_SIZE,"%f",
						(double)EAN(g,STR(enames,i),j));
				agxset(edge,attr,name);
			}
			else
				agxset(edge,attr,(char *)EAS(g,STR(enames,i),j));
		}
	}

	/* write the graph */
	err = agwrite(cg,(void *)output);
d_eit:
	igraph_eit_destroy(&eit);
c_ag:
	agclose(cg);
d_v:
	igraph_vector_ptr_destroy(&vertices);
	return err;
}
