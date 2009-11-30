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
#include "types.hpp"
#include "builtin.hpp"
#include <stdarg.h>

static void print(const char *prefix, const char *mesg, va_list args)
{
	char tab[256];
	vsnprintf(tab,sizeof(tab), mesg, args);
	fprintf(stderr, "%s%s\n",prefix,tab);
}

void die(const char mesg[], ...)
{
	va_list args;

	va_start(args, mesg);
	print("fatal: ", mesg, args);
	va_end(args);
	exit(128);
}

static void print_help(const char *message[])
{
	for(int i = 0; message[i] != NULL; i++)
		fprintf(stderr,"%s",message[i]);
}

void usage(const char *message[]) 
{
	print_help(message);
	exit(129);
}

void usage_helps(int argc, char**argv, help_elt helps[],int ask_full_help)
{
	const char* cmd;
	if(argc > 1)
		cmd = argv[1];
	else
		cmd = helps[0].name;

	if(!strcmp(cmd,helps[0].name))
	{
		print_help(helps[0].help);
		if(ask_full_help)
		{
			std::cout << "\nDetailled help for each command:" << std::endl;
			for(int j = 1; j < ARRAY_SIZE(helps) ; j++)
				print_help(helps[j].help);
		}
		exit(129);
	}

	for(int i = 1; i < ARRAY_SIZE(helps) ; i++)
		if(!strcmp(cmd,helps[i].name))
		{
			print_help(helps[i].help);
			exit(129);
		}

	die("Cannot help on this: %s !",cmd);
}
