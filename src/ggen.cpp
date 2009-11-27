/* Copyright Swann Perarnau 2009
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


#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

/* We use extensively the BOOST library for 
* handling output, program options and random generators
*/

#include "builtin.hpp"

static const char * general_usage_message[] = {
	"Usage: ggen [options] cmd args\n\n",
	"Allowed options:\n",
	"--help                  : produce this help message\n",
	"--version               : launch command in verbose mode\n",
	"\nCommands available:\n",
	"generate-graph          : use the graph utils\n",
	"analyse-graph           : use the graph analysis tools\n",
	"transform-graph         : use the graph transformation tools\n",
	"add-property            : use the property adding tools\n",
	"extract-property        : extract a property from the graph\n",
	NULL
};

void print_usage(const char **message) {
	for(int i=0; message[i] != NULL; i++)
		std::cout << message[i];		
}

static int cmd_help(int argc,char **argv)
{
	print_usage(general_usage_message);
	return 0;
}

static const char *ggen_version_string = PACKAGE_STRING;

static int cmd_version(int argc,char **argv)
{
	std::cout << "ggen: version " << ggen_version_string << std::endl;
	return 0;
}

int main(int argc,char** argv)
{
	static struct cmd_table_elt cmd_table[] = {
		{ "help", cmd_help },
		{ "version", cmd_version },
		{ "generate-graph" , cmd_generate_graph },
		{ "analyse-graph", cmd_analyse_graph },
		{ "transform-graph", cmd_transform_graph },
		{ "add-property", cmd_add_property },
		{ "extract-property", cmd_extract_property },
	};
	const char *cmd;
	
	// get rid of program name
	*argv++;
	argc--;

	// this is the command
	cmd = argv[0];
	if(!cmd)
		cmd = "help";

	// transform --help cmd into cmd --help
	if(argc > 1 && !strcmp(cmd, "--help"))
	{
		cmd = argv[1];
		argv[1] = argv[0];
		*argv++;
		argc--;
	}

	// transform --help and --version into commands
	if(!strncmp(cmd,"--",2))
		cmd += 2;

	// launch command
	for(int i = 0; i < ARRAY_SIZE(cmd_table); i++)
	{
		struct cmd_table_elt *c = cmd_table+i;
		if(!strcmp(c->name,cmd))
		{
			int status = c->fn(argc,argv);
			return status;
		}
	}
	print_usage(general_usage_message);
	return 1;
}
