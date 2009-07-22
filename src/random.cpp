/* Copyright Swann Perarnau 2009
*
*   contributor(s) :  
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
* GGEN GSL Random Number Generator,
* Handle all creation and access to gsl_rng
******************************************************************************/
/* GGEN_RNG_TYPE to gsl_rng_type matrix */

const gsl_rng_type* ggen_rng_table[GGEN_RNG_MAX] = 
{
	gsl_rng_mt19937,
	gsl_rng_ranmar
};

ggen_rng::ggen_rng()
{
	rng = NULL;
}

ggen_rng::~ggen_rng()
{
	gsl_rng_free(rng);
}

int ggen_rng::allocate(const unsigned int rng_type)
{
	if(rng_type < GGEN_RNG_MAX)
	{
		const gsl_rng_type * T = ggen_rng_table[rng_type];
		rng = gsl_rng_alloc(T);
	}
	else 
	{
		std::cerr << "Wrong ggen_rng type asked!" << std::endl;
		return -1;
	}
}

void ggen_rng::seed(unsigned long int s)
{
	gsl_rng_set(rng,s);
}


const gsl_rng* ggen_rng::get_gsl()
{
	return rng;
}


void ggen_rng::read(std::string filename)
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
		std::cerr << "Warning, could not read the rng file. Will not load any rng state" << std::endl;
	}
}

void ggen_rng::write(std::string filename)
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
		std::cerr << "Warning, could not write the rng file. Will not save any rng state" << std::endl;
	}
}

void ggen_rng::choose(boost::any *dest, size_t k, boost::any* src, size_t n, size_t size)
{
	gsl_ran_choose(rng,dest,k,src,n,size);
}

void ggen_rng::shuffle(boost::any *base, size_t n, size_t size)
{
	gsl_ran_shuffle(rng, base, n, size);
}

bool ggen_rng::bernoulli(double p)
{
	if(gsl_ran_bernoulli(rng,p) == 1)
		return true;
	else 
		return false;
}

unsigned long int ggen_rng::uniform_int(unsigned long int n)
{
	return gsl_rng_uniform_int(rng,n);
}


/* GGen RNG Testing code: class used for testing purposes only.
 * Implements all rng functions in a deterministic way
 */
ggen_rng_testing::ggen_rng_testing()
{
	rng = NULL;
	b = true;
}
		
ggen_rng_testing::~ggen_rng_testing()
{
	;
}
		
const gsl_rng* ggen_rng_testing::get_gsl()
{
	return NULL;
}

void ggen_rng_testing::read(std::string filename)
{
	;
}

void ggen_rng_testing::write(std::string filename)
{
	;
}

void ggen_rng_testing::choose(boost::any *dest, size_t k, boost::any* src, size_t n,size_t size)
{
	// copy the first k elements in dest
	// if k > n then copy n elements
	size_t num = k <= n ? k : n;
	num *= size;
	memcpy(dest,src,num);
}

void ggen_rng_testing::shuffle(boost::any *base, size_t n, size_t size)
{
	;
}

bool ggen_rng_testing::bernoulli(double p)
{
	b = !b;
	return !b;
}

unsigned long int ggen_rng_testing::uniform_int(unsigned long int n)
{
	return 0; //TODO: better algorithm
}

/* Base class
 *************/

/* factory method */
ggen_rnd* ggen_rnd::create_rnd(ggen_rng* rng, const unsigned int ggen_rnd_type, std::vector<std::string> args)
{
	switch(ggen_rnd_type)
	{
		case GGEN_RND_GAUSSIAN:
			return new ggen_rnd_gaussian(rng, args);
		case GGEN_RND_FLAT:
			return new ggen_rnd_flat(rng, args);
		default:
			return NULL;
	}
}

ggen_rnd::ggen_rnd(ggen_rng* r)
{
	rng = r;
}


ggen_rnd::~ggen_rnd()
{
	delete rng;
}



/*
*  Gaussian Distribution 
*/

ggen_rnd_gaussian::ggen_rnd_gaussian(ggen_rng* rng, std::vector<std::string> args) : ggen_rnd(rng)
{
	if(args.size() == 1)
		sigma = boost::lexical_cast<double>(args[0]);
	else
		; //TODO error handling + bad lexical cast exception
}

double ggen_rnd_gaussian::get()
{
	return gsl_ran_gaussian(rng->get_gsl(),sigma);
}


/*
*  Flat (Uniform) Distribution
*/

ggen_rnd_flat::ggen_rnd_flat(ggen_rng* rng, std::vector<std::string> args) : ggen_rnd(rng)

{
	if(args.size() == 2)
	{
		min = boost::lexical_cast<double>(args[0]);
		max = boost::lexical_cast<double>(args[1]);
	}
	else
		; //TODO error handling;
}

double ggen_rnd_flat::get()
{
	return gsl_ran_flat(rng->get_gsl(),min,max);
}

///////////////////////////////////////////////////////////////////////////////
// Command line options handling
///////////////////////////////////////////////////////////////////////////////

namespace po =  boost::program_options;

// return the program options allowed for the rng part
po::options_description random_rng_options()
{
	po::options_description desc("Random Number Generator Options");
	desc.add_options()
		("seed,s",po::value<uint64_t>(),"Set the RNG seed")
		("rng-type",po::value<unsigned int>(),"RNG type")
		("rng-file",po::value<std::string>(),"If set, read the RNG state on program start and write it at exit")
		;
	return desc;
}

po::options_description random_rnd_options()
{
	po::options_description desc("Random Number Distribution Options");
	desc.add_options()
		("dist-type",po::value<unsigned int>(),"Random Number Distribution type")
		("dist-args",po::value<std::vector<std::string> >()->multitoken(),"Arguments to the distribution generator")
		;
	return desc;
}

ggen_rng* random_rng_handle_options_atinit(const po::variables_map& vm)
{
	uint64_t seed;
	unsigned int type;
	ggen_rng* rng = new ggen_rng();

	if(vm.count("seed"))
	{
		seed = vm["seed"].as<uint64_t>();
	}
	else
		seed = time(NULL);
	
	if(vm.count("rng-type"))
	{
		type = vm["rng-type"].as<unsigned int>();
	}
	else
		type = GGEN_RNG_DEFAULT;

	rng->allocate(type);
	rng->seed(seed);
	

	// this must be the last, as the rng must have been created
	if(vm.count("rng-file"))
	{
		rng->read(vm["rng-file"].as<std::string>());
	}

	return rng;
}

void random_rng_handle_options_atexit(const po::variables_map& vm,ggen_rng* rng)
{
	if(vm.count("rng-file"))
	{
		rng->write(vm["rng-file"].as<std::string>());
	}
}

ggen_rnd* random_rnd_handle_options_atinit(const po::variables_map& vm,ggen_rng* rng)
{
	unsigned int type;
	std::vector< std::string > args;
	ggen_rnd* rnd;

	if(vm.count("dist-type"))
	{
		type = vm["dist-type"].as<unsigned int>();
	}
	else
		type = GGEN_RND_DEFAULT;

	if(vm.count("dist-args"))
	{
		args = vm["dist-args"].as<std::vector< std::string > >();
	}
	else
	{
		//TODO: this should be a macro
		args.push_back("0");
		args.push_back("100");
	}

	rnd = ggen_rnd::create_rnd(rng,type,args);
	return rnd;
}

void random_rnd_handle_options_atexit(const po::variables_map& vm,ggen_rng* rng, ggen_rnd* rnd)
{
	;
}
