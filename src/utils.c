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
	int e1,e2;
	f = fopen(file,"wb");
	if(!f) return 1;

	e1 = gsl_rng_fwrite(f,*r);
	if(e1) error("GSL error: %s\n",gsl_strerror(e1));

	e2 = fclose(f);
	if(e2) error("I/O error: %s\n",strerror(e2));

	if(e1 || e2)
		return 2;
	else
		return 0;
}

int ggen_rng_load(gsl_rng **r,const char *file)
{
	FILE *f;
	int e1,e2;
	f = fopen(file,"rb");
	if(!f) return 1;

	e1 = gsl_rng_fread(f,*r);
	if(e1) error("GSL error: %s\n",gsl_strerror(e1));

	e2 = fclose(f);
	if(e2) error("I/O error: %s\n",strerror(e2));

	if(e1 || e2)
		return 2;
	else
		return 0;
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
