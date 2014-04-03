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

#include "error.h"
#include <assert.h>

ggen_errno_t ggen_errno;

#define GGEN_DESTRUCTOR_STACK_LENGTH 1000
typedef struct ggen_destructor_stack_st {
	/* contains the next free slot to register a destructor */
	unsigned long sp;
	/* contains the index to the current fp */
	unsigned long sfp;
	/* contains the index to the first registration of this function */
	unsigned long fp[GGEN_DESTRUCTOR_STACK_LENGTH];
	void (*func[GGEN_DESTRUCTOR_STACK_LENGTH])(void *);
	void *obj[GGEN_DESTRUCTOR_STACK_LENGTH];
	int flags[GGEN_DESTRUCTOR_STACK_LENGTH];
} ggen_destructor_stack_t;

static ggen_destructor_stack_t ggen_dstack;

static gsl_error_handler_t *gsl_old_handler;

const char *ggen_error_msgs[] = {
	"success",	
	"failure",
	"internal igraph error",
	"internal GSL error",
	"invalid value",
	"no more memory",
};

const char* ggen_error_strerror(void)
{
	switch(ggen_errno.ggen_error)
	{
		case GGEN_IGRAPH_ERROR:
			return igraph_strerror(ggen_errno.igraph_error);
		case GGEN_GSL_ERROR:
			return gsl_strerror(ggen_errno.gsl_error);
		default:
			return ggen_error_msgs[ggen_errno.ggen_error];
	}
	return NULL;
}

static void ggen_gsl_handler(const char* reason, const char* file, int line, int gsl_errno)
{
	ggen_errno.gsl_error = gsl_errno;
}

int ggen_error_start_stack(void)
{
	assert(ggen_dstack.sfp < GGEN_DESTRUCTOR_STACK_LENGTH-1);
	assert(ggen_dstack.sp < GGEN_DESTRUCTOR_STACK_LENGTH);
	if(ggen_dstack.sfp == 0)
	{
		/* we entered the first ggen function,
		 * install a gsl handler that save the errno
		 */
		gsl_old_handler = gsl_set_error_handler(ggen_gsl_handler);
		ggen_errno.ggen_error = GGEN_SUCCESS;
		ggen_errno.gsl_error = 0;
		ggen_errno.igraph_error = IGRAPH_SUCCESS;
	}
	ggen_dstack.sfp++;
	ggen_dstack.fp[ggen_dstack.sfp] = ggen_dstack.sp;
	return 0;
}

int ggen_error_finally_real(void (*func)(void*), void *ptr, int retval)
{
	assert(ggen_dstack.sp < GGEN_DESTRUCTOR_STACK_LENGTH);
	ggen_dstack.func[ggen_dstack.sp] = func;
	ggen_dstack.obj[ggen_dstack.sp] = ptr;
	ggen_dstack.flags[ggen_dstack.sp] = retval;
	ggen_dstack.sp++;
	return 0;
}

int ggen_error_pop(unsigned long cnt)
{
	for(unsigned long i = 0; i < cnt; i++)
	{
		ggen_dstack.sp--;
		ggen_dstack.func[ggen_dstack.sp](ggen_dstack.obj[ggen_dstack.sp]);
	}
	return 0;
}

int ggen_error_clean(int success)
{
	unsigned long prev_sp = ggen_dstack.fp[ggen_dstack.sfp];
	for(unsigned long i = prev_sp; i < ggen_dstack.sp; i++)
		if(!success || (success && !ggen_dstack.flags[i]))
			ggen_dstack.func[i](ggen_dstack.obj[i]);
	ggen_dstack.sp = prev_sp;
	if(ggen_dstack.sfp > 0)
		ggen_dstack.sfp--;
	if(ggen_dstack.sfp == 0)
	{
		gsl_set_error_handler(gsl_old_handler);
	}
	return 0;
}

