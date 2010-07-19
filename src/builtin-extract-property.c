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

#include <getopt.h>
#include <string.h>

#include "builtin.h"
#include "ggen.h"
#include "utils.h"

static int ask_help = 0;
static char* infile = NULL;
static int is_edge = 0, is_vertex = 1;
static const char* name;

static FILE *in;

static igraph_t g;

static const char* general_help[] = {
	"Usage: ggen analyse-graph [options] name\n",
	"Allowed options:\n",
	"--help               : ask for help\n",
	"--input <filename>   : read the graph for a file\n",
	"--edge,--vertex      : force the type of the property to extract\n",
	"\nArguments:\n",
	"name          : the name of the property to extract\n",
	NULL
};

static struct option long_options[] = {
	{ "help", no_argument, &ask_help, 1 },
	{ "input", required_argument, NULL, 'i' },
	{ "edge", no_argument, &is_edge, 1 },
	{ "vertex", no_argument, &is_vertex, 1 },
	{ 0,0,0,0 },
};

static const char short_opts[] = "hi:";

int cmd_extract_property(int argc, char** argv)
{
	const char* cmd;
	int c;
	int option_index = 0;
	int status = 0;

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

	if(is_edge && is_vertex)
	{
		die("you cannot specify both --edge and --vertex");
	}

	// this is the name
	name = argv[0];
	if(!name)
		ask_help = 1;

	if(ask_help || !strcmp(name,"help"))
	{
		usage(general_help);
		goto ret;
	}

	// read graph
	if(infile)
	{
		fprintf(stderr,"Using %s as graph file\n",infile);
		in = fopen(infile,"r");
		if(!in)
		{
			fprintf(stderr,"failed to open file %s for graph input\n",infile);
			status = 1;
			goto ret;
		}
		status = ggen_read_graph(&g,in);
		fclose(in);
		if(status) goto cleanup;
	}

	unsigned long count,i,v;
	if(is_edge)
		count = igraph_ecount(&g);
	else
		count = igraph_vcount(&g);

	for(i = 0; i < count; i++)
	{
		if(is_edge)
			fprintf(stdout,"%lu,%s",i,EAS(&g,name,i));
		else
			fprintf(stdout,"%lu,%s",i,VAS(&g,name,i));
	}
	fprintf(stdout,"\n");
cleanup:
	igraph_destroy(&g);
ret:
	return status;
}
