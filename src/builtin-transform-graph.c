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

#include <fcntl.h>
#include <getopt.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "builtin.h"
#include "ggen.h"
#include "utils.h"

const char* help_transform[] = {
	"Commands:\n",
	"remove-sinks            : remove all sinks present in the graph\n",
	"remove-sources          : remove all sources present in the graph\n",
	"add-sink   <name>       : add a named node connected to all previous sinks\n",
	"add-source <name>       : add a named node connected to all previous sources\n",
	"transitive-closure      : make the transitive closure of the graph\n",
	NULL,
};

static int cmd_remove_sinks(int argc, char** argv)
{
	return ggen_transform_delete(&g,GGEN_TRANSFORM_SINK);
}

static int cmd_remove_sources(int argc, char** argv)
{
	return ggen_transform_delete(&g,GGEN_TRANSFORM_SOURCE);
}

static int cmd_add_sink(int argc, char** argv)
{
	int err = ggen_transform_add(&g,GGEN_TRANSFORM_SINK);
	if(err) return err;

	SETVAS(&g,GGEN_VERTEX_NAME_ATTR,igraph_vcount(&g)-1,argv[0]);
	return 0;
}

static int cmd_add_source(int argc, char** argv)
{
	int err = ggen_transform_add(&g,GGEN_TRANSFORM_SOURCE);
	if(err) return err;

	SETVAS(&g,GGEN_VERTEX_NAME_ATTR,igraph_vcount(&g)-1,argv[0]);
	return 0;
}

static int cmd_transitive_closure(int argc, char **argv)
{
	return ggen_transform_transitive_closure(&g);
}

struct second_lvl_cmd cmds_transform[] = {
	{ "remove-sinks", 0, NULL, cmd_remove_sinks },
	{ "remove-sources", 0, NULL, cmd_remove_sources },
	{ "add-sink", 1, NULL, cmd_add_sink },
	{ "add-source", 1, NULL, cmd_add_source },
	{ "transitive-closure", 0, NULL, cmd_transitive_closure },
	{ 0, 0, 0, 0},
};
