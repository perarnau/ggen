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

static int ask_help = 0;
static char* infile = NULL;

static igraph_t g;

/* usage and cmd functions */
static const char* general_help[] = {
	"Usage: ggen analyse-graph [options] cmd\n",
	"Allowed options:\n",
	"--help               : ask for help\n",
	"--input <filename>   : read the graph from a file\n",
	"\nAllowed commands:\n",
	"nb-vertices          : gives the number of vertices in the graph\n",
	"nb-edges             : gives the number of edges in the graph\n",
	"mst                  : compute the Minimum Spanning Tree of the graph\n",
	"lp                   : compute the Longest Path of the graph\n",
	"out-degree           : gives the out_degree of each vertex\n",
	"in-degree            : gives the in_degree of each vertex\n",
	"max-independent-set  : gives a maximum independent set of the graph\n",
	"strong-components    : gives the list of all strong components of the graph\n",
	"maximal-paths        : gives the list of all maximal paths (ending by a sink)\n",
	NULL,
};

static int cmd_help(int argc, char **argv)
{
	usage(general_help);
	return 0;
}

static int cmd_nb_vertices(int argc, char **argv)
{
	fprintf(stdout,"Number of vertices: %lu\n",(unsigned long)igraph_vcount(&g));
	return 0;
}

static int cmd_nb_edges(int argc, char **argv)
{
	fprintf(stdout,"Number of vertices: %lu\n",(unsigned long)igraph_ecount(&g));
	return 0;
}

static int cmd_mst(int argc, char **argv)
{
	int err;
	igraph_t mst;
	err = igraph_minimum_spanning_tree_unweighted(&g,&mst);
	if(err) return err;

	err = ggen_write_graph(&g,stdout);
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
		fprintf(stdout,"%lu",(unsigned long)VECTOR(*lp)[i]);
	}
	fprintf(stdout,"\n");

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
		fprintf(stdout,"%lu,%lu\n",i,(unsigned long)VECTOR(d)[i]);
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
		fprintf(stdout,"%lu,%lu\n",i,(unsigned long)VECTOR(d)[i]);
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
		fprintf(stdout,"%lu",(unsigned long)VECTOR(l)[i]);
	}
	fprintf(stdout,"\n");
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

	fprintf(stdout,"Nb of strong components: %lu\n",(unsigned long)n);
	return 0;
}

static struct cmd_table_elt cmd_table[] = {
	{ "help", cmd_help },
	{ "nb-vertices", cmd_nb_vertices },
	{ "nb-edges",   cmd_nb_edges },
        { "mst", cmd_mst },
        { "lp", cmd_lp },
        { "out-degree", cmd_out_degree },
        { "in-degree", cmd_in_degree },
        { "max-independent-set", cmd_max_indep_set },
        { "strong-components", cmd_strong_components },
};

static struct option long_options[] = {
	{ "help", no_argument, &ask_help, 1 },
	{ "input", required_argument, NULL, 'i' },
	{ 0,0,0,0 },
};

static const char short_opts[] = "hi:";

int cmd_analyse_graph(int argc,char** argv)
{
	const char* cmd;
	int c,err;
	int option_index = 0;
	FILE * in;
	while(1)
	{
		c = getopt_long(argc, argv, short_opts,long_options, &option_index);
		if(c == -1)
			break;

		switch(c)
		{
			case 'h':
			case 0:
				break;
			case 'i':
				infile = optarg;
				break;
			case '?':
			default:
				exit(1);
		}
	}

	// now forget the parsed part of argv
	argc -= optind;
	argv = &(argv[optind]);

	// this is the command
	cmd = argv[0];
	if(!cmd)
		cmd = "help";

	int status = 0;
	if(!strcmp(cmd,"help") || ask_help)
	{
		usage(general_help);
		goto ret;
	}

	for(int i = 1; i < ARRAY_SIZE(cmd_table); i++)
	{
		struct cmd_table_elt *c = cmd_table+i;
		if(!strcmp(c->name,cmd))
		{
			// read graph
			if(infile)
			{
				fprintf(stdout,"Using %s as graph file\n",infile);
				in = fopen(infile,"r");
				if(!in)
				{
					fprintf(stderr,
						"failed to open file %s for graph input\n",
						infile);
					status = 1;
					goto ret;
				}
			}
			else {
				fprintf(stdout,"Using standard input\n");
				in = stdin;
			}
			status = ggen_read_graph(&g,in);
			if(infile)
				fclose(in);
			if(status) goto ret;
			status = c->fn(argc,argv);
			goto end;
		}
	}
	fprintf(stderr,"Wrong command\n");
	status = 1;
end:
	igraph_destroy(&g);
ret:
	return status;
}
