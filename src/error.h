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
#ifndef ERROR_H
#define ERROR_H 1

/* error management for the ggen library.
 * This code is the current facility to tackle the most annoying part of ggen:
 * error management and reporting of the underlying libraries.
 *
 * The error management is organised as follows:
 *
 * - All error checking jumps (goto) an error label named 'ggen_error_label' 
 * - GGen keep track of real error codes in a static structure named ggen_errno.
 * - functions are provided to print, check, and cleanup errors.
 *
 * The cleanup part is heavily inspired by igraph:
 * - a stack of destructors is maintained during execution
 * - functions push destructors to correctly initialized objects
 * - before returning, the stacked destructors are called.
 * - an error triggers immediate destruction, and return an error.
 * Contrary to igraph, an error doesn't unwind the allocation stack. Callers
 * are always responsible for current live objects.
 */

#include <igraph/igraph_error.h>
#include <gsl/gsl_errno.h>

typedef enum {
	GGEN_SUCCESS	= 0,
	GGEN_FAILURE	= 1,
	GGEN_IGRAPH_ERROR = 2,
	GGEN_GSL_ERROR = 3,
	GGEN_EINVAL = 4,
	GGEN_ENOMEM = 5,
} ggen_error_type_t;

typedef struct ggen_errno_st {
	ggen_error_type_t ggen_error;
	igraph_error_type_t igraph_error;
	unsigned long gsl_error;
} ggen_errno_t;

extern ggen_errno_t ggen_errno;

/* check for an error when calling igraph */
#define GGEN_CHECK_IGRAPH(x) do {				\
	int ggen_i_ret = (x);					\
	if(ggen_i_ret != IGRAPH_SUCCESS)			\
	{							\
		ggen_errno.igraph_error = ggen_i_ret;		\
		ggen_errno.ggen_error = GGEN_IGRAPH_ERROR;	\
		ggen_error_clean(0);				\
		goto ggen_error_label;				\
	}							\
} while(0)

/* check function used for vector_ptr:
 * a vector of pointer is handled by creating it, setting the item destructor
 * and then allocating and initializing each element. Thus, if the
 * initialization of an element fails, we first have to free this element
 * before the vector destructor call be called.
 * x is the init function, v the vector_ptr, i the element
 */
#define GGEN_CHECK_IGRAPH_VECTPTR(x,v,i) do {			\
	int ggen_i_ret = (x);					\
	if(ggen_i_ret != IGRAPH_SUCCESS)			\
	{							\
		ggen_errno.igraph_error = ggen_i_ret;		\
		ggen_errno.ggen_error = GGEN_IGRAPH_ERROR;	\
		free(VECTOR(v)[i]);				\
		VECTOR(v)[i] = NULL;				\
		ggen_error_clean(0);				\
		goto ggen_error_label;				\
	}							\
} while(0)


#define GGEN_CHECK_GSL(x) do {				\
	int ggen_i_ret = (x);				\
	if(ggen_i_ret != 0)				\
	{						\
		ggen_errno.gsl_error = ggen_i_ret;	\
		ggen_errno.ggen_error = GGEN_GSL_ERROR;	\
		ggen_error_clean(0);			\
		goto ggen_error_label;			\
	}						\
} while(0)

/* make the test, but in case of errors, we already saved the errno in the
 * struct using our handler.
 * the test is like an assert, it should return 1 on success
 */
#define GGEN_CHECK_GSL_ERRNO(x) do {			\
	int ggen_i_ret = (x);				\
	if(!ggen_i_ret)					\
	{						\
		ggen_errno.ggen_error = GGEN_GSL_ERROR;	\
		ggen_error_clean(0);			\
		goto ggen_error_label;			\
	}						\
} while(0)

/* do instruction, and check gsl errno directly,
 * saved the errno in the  struct using our handler in case of error
 */
#define GGEN_CHECK_GSL_DO(x) do {			\
	ggen_errno.gsl_error = 0;			\
	(x);						\
	if(ggen_errno.gsl_error != 0)			\
	{						\
		ggen_errno.ggen_error = GGEN_GSL_ERROR;	\
		ggen_error_clean(0);			\
		goto ggen_error_label;			\
	}						\
} while(0)

#define GGEN_CHECK_INTERNAL(x) do {			\
	int ggen_i_ret = (x);				\
	if(ggen_i_ret != GGEN_SUCCESS)			\
	{						\
		ggen_errno.ggen_error = ggen_i_ret;	\
		ggen_error_clean(0);			\
		goto ggen_error_label;			\
	}						\
} while(0)

#define GGEN_CHECK_INTERNAL_ERRNO(x) do {	\
	int ggen_i_ret = (x);			\
	if(ggen_i_ret != GGEN_SUCCESS)		\
	{					\
		ggen_error_clean(0);		\
		goto ggen_error_label;		\
	}					\
} while(0)

#define GGEN_CHECK_INTERNAL_DO(x) do {			\
	ggen_errno.ggen_error = GGEN_SUCCESS;		\
	(x);						\
	if(ggen_errno.ggen_error != GGEN_SUCCESS)	\
	{						\
		ggen_error_clean(0);			\
		goto ggen_error_label;			\
	}						\
} while(0)

#define GGEN_CHECK_ALLOC(x) do {			\
	if((x) == NULL)					\
	{						\
		ggen_errno.ggen_error = GGEN_ENOMEM;	\
		ggen_error_clean(0);			\
		goto ggen_error_label;			\
	}						\
} while(0)

#define GGEN_SET_ERRNO(x) do {			\
	ggen_errno.ggen_error = x;		\
	ggen_error_clean(0);			\
	goto ggen_error_label;			\
} while(0)

#define GGEN_FINALLY(f,p) ggen_error_finally_real((void (*)(void*))f, (void*)p,0)
#define GGEN_FINALLY3(f,p,r) ggen_error_finally_real((void (*)(void*))f, (void*)p,r)
const char* ggen_error_strerror(void);

/* start a new stack of destructor pointers */
int ggen_error_start_stack(void);

/* register a destructor to be called on ggen_error_clean */
int ggen_error_finally_real(void (*func)(void*), void *ptr, int returnval);

/* pop and call a number of destructor from the stack */
int ggen_error_pop(unsigned long);

/* pop all the current stack of destructor, calling them.
 * if success is true, we avoid calling the destructors on objects flagged as
 * return values
 */
int ggen_error_clean(int success);

#endif // ERROR_H
