/* GGen is a random graph generator :
 * it provides means to generate a graph following a
 * collection of methods found in the litterature.
 *
 * This is a research project founded by the MOAIS Team,
 * INRIA, Grenoble Universities.
 */

#include "random.hpp"
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

/* This file is the gls random number distribution wrapper for ggen,
 * It is designed to handle the creation, and "normal" utilization of
 * gsl for ggen, according to user defined command line argument.
 */

/******************************************************************************
  * GGEN GSL Random Number Distribution,
  * Factory pattern
  * Handle all creation and access to gsl_rnd
******************************************************************************/

/* Base class
 *************/

/* factory method */
ggen_rnd* ggen_rnd::create_rnd(const unsigned int ggen_rnd_type, const unsigned int rng_type, unsigned long int seed, void *parameters[3])
{
	switch(ggen_rnd_type)
	{
		case GGEN_RND_GAUSSIAN:
			return new ggen_rnd_gaussian(rng_type, seed, parameters);
		case GGEN_RND_FLAT:
			return new ggen_rnd_flat(rng_type, seed, parameters);
		default:
			return NULL;
	}
}

/* this constructor allows centralized creation of the rng associated with each rnd */
ggen_rnd::ggen_rnd(const unsigned int rng_type, unsigned long int seed)
{
	const gsl_rng_type * T = ggen_rng_table[rng_type];
	rng = gsl_rng_alloc(T);
	gsl_rng_set(rng,seed);
}

/* base class destructor */
ggen_rnd::~ggen_rnd()
{
	gsl_rng_free(rng);
}


/* Gaussian Distribution 
 ************************/

ggen_rnd_gaussian::ggen_rnd_gaussian(const unsigned int rng_type, unsigned long int seed, void *parameters[3]) : ggen_rnd(rng_type,seed)
{
	sigma = *((double*)parameters[1]);
}

double ggen_rnd_gaussian::get()
{
	return gsl_ran_gaussian(rng,sigma);
}


/* Flat (Uniform) Distribution 
 ******************************/

ggen_rnd_flat::ggen_rnd_flat(const unsigned int rng_type, unsigned long int seed, void *parameters[3]) : ggen_rnd(rng_type,seed)

{
	min = *((double*)parameters[1]);
	max = *((double*)parameters[2]);
}

double ggen_rnd_flat::get()
{
	return gsl_ran_flat(rng,min,max);
}


