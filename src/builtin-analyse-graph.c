/* Copyright Swann Perarnau 2009
 *
 *   contributor(s) :
 *
 *   contact : firstname.lastname@imag.fr
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

#include <getopt.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "builtin.h"
#include "ggen.h"
#include "utils.h"

/* usage and cmd functions */
const char* help_analyse[] = {
	"Commands:\n",
	"nb-vertices          : gives the number of vertices in the graph\n",
	"nb-edges             : gives the number of edges in the graph\n",
	"mst                  : compute the Minimum Spanng Tree of the graph\n",
	"lp                   : compute the Longest Path of the graph\n",
	"out-degree           : gives the out_degree of each vertex\n",
	"in-degree            : gives the in_degree of each vertex\n",
	"max-independent-set  : gives a maximum independent set of the graph\n",
	"strong-components    : gives the list of all strong components of the graph\n",
	"maximal-paths        : gives the list of all maximal paths (endg by a sink)\n",
	NULL,
};

static int cmd_nb_vertices(int argc, char **argv)
{
	fprintf(outfile,"Number of vertices: %lu\n",(unsigned long)igraph_vcount(&g));
	return 0;
}

static int cmd_nb_edges(int argc, char **argv)
{
	fprintf(outfile,"Number of vertices: %lu\n",(unsigned long)igraph_ecount(&g));
	return 0;
}

static int cmd_mst(int argc, char **argv)
{
	int err;
	igraph_t mst;
	err = igraph_minimum_spanning_tree_unweighted(&g,&mst);
	if(err) return err;

	err = ggen_write_graph(&mst,outfile);
	igraph_destroy(&mst);
	return err;
}

static int cmd_lp(int argc, char **argv)
{
	int err = 0;
	unsigned long i = 0;
	igraph_vector_t *lp = NULL;
	lp = ggen_analyze_longest_path(&g);
	if(!lp) return 1;

	for(i = 0; i < igraph_vector_size(lp); i++)
	{
		fprintf(outfile,"%lu",(unsigned long)VECTOR(*lp)[i]);
	}
	fprintf(outfile,"\n");

	igraph_vector_destroy(lp);
	free(lp);
	return 0;
}

static int cmd_out_degree(int argc, char **argv)
{
	int err = 0;
	unsigned long i;
	igraph_vector_t d;
	err = igraph_vector_init(&d,igraph_vcount(&g));
	if(err) goto ret;

	err = igraph_degree(&g,&d,igraph_vss_all(),IGRAPH_OUT,0);
	if(err) goto error;

	for(i = 0; i < igraph_vcount(&g); i++)
	{
		fprintf(outfile,"%lu,%lu\n",i,(unsigned long)VECTOR(d)[i]);
	}
error:
	igraph_vector_destroy(&d);
ret:
	return err;
}

static int cmd_in_degree(int argc, char **argv)
{
	int err = 0;
	unsigned long i = 0;
	igraph_vector_t d;
	err = igraph_vector_init(&d,igraph_vcount(&g));
	if(err) goto ret;

	err = igraph_degree(&g,&d,igraph_vss_all(),IGRAPH_IN,0);
	if(err) goto error;

	for(i = 0; i < igraph_vcount(&g); i++)
	{
		fprintf(outfile,"%lu,%lu\n",i,(unsigned long)VECTOR(d)[i]);
	}
error:
	igraph_vector_destroy(&d);
ret:
	return err;
}

static int cmd_max_indep_set(int argc, char **argv)
{
	int err = 0;
	unsigned long i = 0;
	igraph_vector_ptr_t l;

	err = igraph_vector_ptr_init(&l,igraph_vcount(&g));
	if(err) return 1;

	err = igraph_largest_independent_vertex_sets(&g,&l);
	if(err) goto error;

	for(i = 0; i < igraph_vector_ptr_size(&l); i++)
	{
		fprintf(outfile,"%lu",(unsigned long)VECTOR(l)[i]);
	}
	fprintf(outfile,"\n");
error:
	igraph_vector_ptr_destroy(&l);
	return err;
}
static int cmd_strong_components(int argc, char **argv)
{
	int err;
	igraph_integer_t n;
	err = igraph_clusters(&g,NULL,NULL,&n,IGRAPH_STRONG);
	if(err) return err;

	fprintf(outfile,"Nb of strong components: %lu\n",(unsigned long)n);
	return 0;
}

struct second_lvl_cmd  cmds_analyse[] = {
	{ "nb-vertices", 0, NULL, cmd_nb_vertices },
	{ "nb-edges", 0, NULL, cmd_nb_edges },
        { "mst", 0, NULL, cmd_mst },
        { "lp", 0, NULL, cmd_lp },
        { "out-degree", 0, NULL, cmd_out_degree },
        { "in-degree", 0, NULL, cmd_in_degree },
        { "max-independent-set", 0, NULL, cmd_max_indep_set },
        { "strong-components", 0, NULL, cmd_strong_components },
	{ 0, 0, 0, 0},
};
