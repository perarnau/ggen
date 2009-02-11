/* GGen is a random graph generator :
 * it provides means to generate a graph following a
 * collection of methods found in the litterature.
 *
 * This is a research project founded by the MOAIS Team,
 * INRIA, Grenoble Universities.
 */

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

/* This file is the gls random number distribution wrapper for ggen,
 * It is designed to handle the creation, and "normal" utilization of
 * gsl for ggen, according to user defined command line argument.
 */

/* First thing to do it to define the complete collection of supported random number
 * distributions */

#define GGEN_RND_GAUSSIAN 1
#define GGEN_RND_FLAT 2

#define GGEN_RND_DEFAULT GGEN_RND_FLAT


/* GSL RNG types */

#define GGEN_RNG_MT19937 1

#define GGEN_RNG_DEFAULT GGEN_RNG_MT19937

/* GGEN_RNG_TYPE to gsl_rng_type matrix */

const gsl_rng_type* ggen_rng_table[1] = 
{
	gsl_rng_mt19937
};



/******************************************************************************
 * GGEN GSL Random Number Distribution,
 * Factory pattern
 * Handle all creation and access to gsl_rnd
******************************************************************************/

/* This ggen_rnd class is responsible of the creation of all supported random number distribution
 * It contains a protected random number generator common to all derived classes as all gsl_rnd
 * need a gsl_rng to work.
 */


class ggen_rnd {
	public:
		ggen_rnd(const unsigned int rng_type, unsigned long int seed);
		virtual double get();
		~ggen_rnd();

		static ggen_rnd* create_rnd(const unsigned int ggen_rnd_type, const unsigned int rng_type, unsigned long int seed, void *parameters[3]);

	protected:
		gsl_rng *rng;
};

/* Gaussian distribution
 ************************/

class ggen_rnd_gaussian : public ggen_rnd {
	public:
		ggen_rnd_gaussian(const unsigned int rng_type, unsigned long int seed, void *parameters[3]);
		double get();
	private:
		double sigma;
};

/* Flat (unifrom) distribution
 ******************************/

class ggen_rnd_flat : public ggen_rnd {
	public:
		ggen_rnd_flat(const unsigned int rng_type, unsigned long int seed, void *parameters[3]);
		double get();
	private:
		double min,max;
};
