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

#include <string.h>
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_histogram.h>
#include "builtin.h"
#include "ggen.h"
#include "utils.h"

const char* help_analyse_prop[] = {
	"Commands:\n",
	"print                       : just print the property\n",
	"stats                       : print the mean and stddev of the property\n",
	"hist <nbins> <xmin> <xmax>  : print an histogram of the property\n",
	NULL
};

/* Searches the attribute attr_name, of ptype attr_type in ig.
 * returns 0 if the attribute exists and is a string attribute
 * returns 1 if the attribute exists and is a numerical attribute
 * return -1 if the attribute cannot be found or an error occurred.
 */
int find_attribute(igraph_t *ig,int attr_type,char *attr_name)
{
	int err,i;
	igraph_strvector_t gnames,vnames,enames;
	igraph_vector_t gtypes,vtypes,etypes;
	unsigned long vcount,ecount;
	vcount = igraph_vcount(ig);
	ecount = igraph_ecount(ig);
	igraph_strvector_init(&gnames,1);
	igraph_strvector_init(&vnames,vcount);
	igraph_strvector_init(&enames,ecount);
	igraph_vector_init(&gtypes,1);
	igraph_vector_init(&vtypes,vcount);
	igraph_vector_init(&etypes,ecount);

	err = igraph_cattribute_list(ig,&gnames,&gtypes,&vnames,&vtypes,&enames,&etypes);
	if(err) return -1;

	err = -1;
	if(attr_type == EDGE_PROPERTY)
	{
		for(i = 0; i < igraph_strvector_size(&enames); i++)
		{
			if(!strcmp((char*)STR(enames,i),attr_name))
			{
				if(VECTOR(etypes)[i]==IGRAPH_ATTRIBUTE_NUMERIC)
					err = 1;
				else
					err = 0;
			}
		}
	}
	else if(attr_type == VERTEX_PROPERTY)
	{
		for(i = 0; i < igraph_strvector_size(&vnames); i++)
		{
			if(!strcmp((char*)STR(vnames,i),attr_name))
			{
				if(VECTOR(vtypes)[i]==IGRAPH_ATTRIBUTE_NUMERIC)
					err = 1;
				else
					err = 0;
			}
		}
	}
	return err;
}


int cmd_print(int argc, char **argv)
{
	unsigned long count;
	int attr_type;
	char n[GGEN_DEFAULT_NAME_SIZE];
	char *s;
	attr_type = find_attribute(&g,ptype,name);
	if(attr_type == -1)
	{
		error("error: could not find property (%s)\n",name);
		return 1;
	}
	if(ptype == EDGE_PROPERTY)
	{
		count = igraph_ecount(&g);
		for(unsigned long i = 0; i < count; i++)
		{
			if(attr_type == 0)
				fprintf(outfile,"%lu,%s\n",i,EAS(&g,name,i));
			else
				fprintf(outfile,"%lu,%f\n",i,(double)EAN(&g,name,i));
		}
	}
	else if(ptype == VERTEX_PROPERTY)
	{
		count = igraph_vcount(&g);
		for(unsigned long i = 0; i < count; i++)
		{
			s = ggen_vname(n,&g,i);
			if(attr_type == 0)
				fprintf(outfile,"%s,%s\n",s==NULL?n:s,VAS(&g,name,i));
			else
				fprintf(outfile,"%s,%f\n",s==NULL?n:s,(double)VAN(&g,name,i));
		}
	}
	return 0;
}

size_t get_property_size(igraph_t *ig,char *pname, int type)
{
	if(type == EDGE_PROPERTY)
		return igraph_ecount(ig);
	else if(type == VERTEX_PROPERTY)
		return igraph_vcount(ig);
	else
		return 0;
}

int get_property(igraph_t *ig, double *dest, char *pname, int attr_type, int ptype, int index)
{
	if(attr_type == 0)
	{
		if(ptype == EDGE_PROPERTY)
			return s2d((char*)EAS(ig,pname,index),dest);
		else
			return s2d((char*)VAS(ig,pname,index),dest);
	}
	else
	{
		if(ptype == EDGE_PROPERTY)
		{
			*dest = (double) EAN(ig,pname,index);
			return 0;
		}
		else
		{
			*dest = (double) VAN(ig,pname,index);
			return 0;
		}
	}
}

int cmd_stats(int argc, char **argv)
{
	int attr_type,i,size;
	double *values;
	double mean,sd;
	int err = 0;
	attr_type = find_attribute(&g,ptype,name);
	if(attr_type == -1)
	{
		error("error: could not find property (%s)\n",name);
		return 1;
	}
	size = get_property_size(&g,name,ptype);
	values = calloc(size,sizeof(double));
	if(values == NULL)
	{
		error("error: failed allocation\n");
		return 1;
	}

	for(i = 0; i < size; i++)
	{
		err = get_property(&g,&values[i],name,attr_type,ptype,i);
		if(err) goto free_val;
	}
	// now that our array is full, make a lot of stats
	mean = gsl_stats_mean(values,1,size);
	sd = gsl_stats_sd_m(values,1,size,mean);
	fprintf(outfile,"mean: %f\n",mean);
	fprintf(outfile,"sd: %f\n",sd);
free_val:
	free(values);
	return err;
}

int cmd_hist(int argc, char **argv)
{
	int err,i;
	int attr_type;
	gsl_histogram *h = NULL;
	double xmin,xmax,tmp;
	unsigned long size;
	unsigned long count;
	attr_type = find_attribute(&g,ptype,name);
	if(attr_type == -1)
	{
		error("error: could not find property (%s)\n",name);
		return 1;
	}
	else if(attr_type == 0)
	{
		error("warning: property is a string, will continue anyway\n");
	}
	err = s2ul(argv[0],&size);
	if(err)
	{
		error("error: parsing first argument\n");
		return 1;
	}
	err = s2d(argv[1],&xmin);
	if(err)
	{
		error("error: parsing second argument\n");
		return 1;
	}
	err = s2d(argv[2],&xmax);
	if(err)
	{
		error("error: parsing third argument\n");
		return 1;
	}
	// allocate histogram
	h = gsl_histogram_alloc(size);
	if(h == NULL)
	{
		error("error: allocating histogram\n");
		return 1;
	}
	gsl_histogram_set_ranges_uniform(h,xmin,xmax);
	// now fill it
	count = get_property_size(&g,name,ptype);
	for(i = 0; i < count; i++)
	{
		err = get_property(&g,&tmp,name,attr_type,ptype,i);
		if(err) goto free_h;
		err = gsl_histogram_increment(h,tmp);
		if(err)
		{
			error("error during histogram increment, probably an out of range value\n");
			goto free_h;
		}
	}
	// and print
	gsl_histogram_fprintf(outfile,h,"%f","%f");
free_h:
	gsl_histogram_free(h);
	return err;
}

struct second_lvl_cmd cmds_analyse_prop[] = {
	{ "print", 0, NULL, cmd_print },
	{ "stats", 0, NULL, cmd_stats },
	{ "hist", 3, NULL, cmd_hist },
	{ 0, 0, 0, 0},
};
