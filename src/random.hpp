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

#ifndef RANDOM_HPP
#define RANDOM_HPP

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <iostream>
#include <boost/config.hpp>
#include <boost/program_options.hpp>
/* This file is the gls random number distribution wrapper for ggen,
 * It is designed to handle the creation, and "normal" utilization of
 * gsl for ggen, according to user defined command line argument.
 */

/* First thing to do it to define the complete collection of supported random number
 * distributions */

#define GGEN_RND_GAUSSIAN 	0
#define GGEN_RND_FLAT 		1
#define GGEN_RND_MAX 		2

#define GGEN_RND_DEFAULT GGEN_RND_FLAT


/* GSL RNG types */

#define GGEN_RNG_MT19937	0
#define GGEN_RNG_RANMAR		1
#define GGEN_RNG_MAX		2


#define GGEN_RNG_DEFAULT GGEN_RNG_RANMAR

/******************************************************************************
 * GGEN GSL Random Number Generator,
 * Handle all creation and access to gsl_rng
******************************************************************************/

class ggen_rng {
	public:
		ggen_rng(const unsigned int rng_type, unsigned long int seed);
		~ggen_rng();
		const gsl_rng* get_gsl();
		void read(std::string filename);
		void write(std::string filename);
		void choose(boost::any *dest, size_t k, boost::any* src, size_t n,size_t size);
		void shuffle(boost::any *base, size_t n, size_t size);
		bool bernoulli(double p = 0.5);
		unsigned long int uniform_int(unsigned long int n);
	protected:
		gsl_rng* rng;
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
		ggen_rnd(ggen_rng* r);
		virtual double get() = 0;
		~ggen_rnd();		
		static ggen_rnd* create_rnd(ggen_rng* rng, const unsigned int ggen_rnd_type, std::vector<std::string> args); 
	
	protected:
		ggen_rng* rng;
};

/* Gaussian distribution
 ************************/

class ggen_rnd_gaussian : public ggen_rnd {
	public:
		ggen_rnd_gaussian(ggen_rng* rng, std::vector<std::string> args);
		double get();
	private:
		double sigma;
};

/* Flat (unifrom) distribution
 ******************************/

class ggen_rnd_flat : public ggen_rnd {
	public:
		ggen_rnd_flat(ggen_rng* rng, std::vector<std::string> args);
		double get();
	private:
		double min,max;
};

///////////////////////////////////////////////////////////////////////////////
// Command line options handling
///////////////////////////////////////////////////////////////////////////////

namespace po =  boost::program_options;

po::options_description random_rng_options();
po::options_description random_rnd_options();
ggen_rng* random_rng_handle_options_atinit(const po::variables_map& vm);
void random_rng_handle_options_atexit(const po::variables_map& vm,ggen_rng* rng);
ggen_rnd* random_rnd_handle_options_atinit(const po::variables_map& vm,ggen_rng* rng);
void random_rnd_handle_options_atexit(const po::variables_map& vm,ggen_rng* rng, ggen_rnd* rnd);

#endif
