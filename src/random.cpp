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
#include <cstdio>
#include <boost/config.hpp>
#include <boost/program_options.hpp>

/* This file is the gls random number distribution wrapper for ggen,
 * It is designed to handle the creation, and "normal" utilization of
 * gsl for ggen, according to user defined command line argument.
 */

/******************************************************************************
  * GGEN GSL Random Number Distribution,
  * Factory pattern
  * Handle all creation and access to gsl_rnd
******************************************************************************/

/* GGEN_RNG_TYPE to gsl_rng_type matrix */

const gsl_rng_type* ggen_rng_table[GGEN_RNG_MAX] = 
{
	gsl_rng_mt19937,
	gsl_rng_ranmar
};

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

ggen_rnd* ggen_rnd::create_rnd(const unsigned int rng_type, unsigned long int seed)
{
	double min = 0.0;
	double max = 1.0;
	void *p[3] = { &min, &max, NULL };
	return create_rnd(GGEN_RND_DEFAULT,rng_type,seed,p);
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

/* this method reads the gsl_rng state from a file */
void ggen_rnd::read(std::string filename)
{
	FILE *input = NULL;
	input = fopen(filename.c_str(),"r");
	if(input != NULL)
	{
		gsl_rng_fread(input,rng);
		fclose(input);
	}
	else
	{
		// TODO : error handling
	}
}

/* this method writes the gsl_rng state from a file */
void ggen_rnd::write(std::string filename)
{
	FILE *output = NULL;
	output = fopen(filename.c_str(),"w");
	if(output != NULL)
	{
		gsl_rng_fwrite(output,rng);
		fclose(output);
	}
	else
	{
		// TODO : error handling
	}
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
	min = *((double*)parameters[0]);
	max = *((double*)parameters[1]);
}

double ggen_rnd_flat::get()
{
	return gsl_ran_flat(rng,min,max);
}

///////////////////////////////////////////////////////////////////////////////
// Command line options handling
///////////////////////////////////////////////////////////////////////////////

namespace po =  boost::program_options;

/*
struct random_options_state {
	unsigned int rng_type;
	ggen_rnd *rnd;
	std::string filename;
	unsigned int seed;
}
*/

// return the program options allowed for the rng/rnd part
po::options_description random_add_options()
{
	po::options_description desc("Random Number Generation");
	desc.add_options()
		("seed,s",po::value<uint64_t>(),"Set the RNG seed")
		("rng-type",po::value<unsigned int>(),"RNG type")
		("rng-file",po::value<std::string>(),"If set, read the RNG state on program start and write it at exit")
		("dist-type",po::value<unsigned int>(),"Random Number Distribution type")
	;

	return desc;
}

void random_options_start(const po::variables_map& vm,random_options_state& rs)
{
	if(vm.count("seed"))
	{
		rs.seed = vm["seed"].as<uint64_t>();
	}
	else
		rs.seed = time(NULL);

	if(vm.count("rng-type"))
	{
		rs.rng_type = vm["rng-type"].as<unsigned int>();
		if(rs.rng_type >= GGEN_RNG_MAX)
			std::cout << "Error : invalid rng-type option, max is : " << GGEN_RNG_MAX-1 << ".\n";
	}
	else
		rs.rng_type = GGEN_RNG_DEFAULT;

	
	if(vm.count("dist-type"))
	{
		//TODO
		;
	}
	else
		rs.rnd = ggen_rnd::create_rnd(rs.rng_type,rs.seed);


	// this must be the last, as the rnd must have been created
	if(vm.count("rng-file"))
	{
		rs.rnd->read(vm["rng-file"].as<std::string>());
	}
}

void random_options_end(const po::variables_map& vm,random_options_state& rs)
{
	if(vm.count("rng-file"))
	{
		rs.rnd->write(vm["rng-file"].as<std::string>());
	}
}

