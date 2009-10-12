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
#include <boost/any.hpp>

namespace ggen {

/* This file is the gls random number distribution wrapper for ggen,
 * It is designed to handle the creation, and "normal" utilization of
 * gsl for ggen, according to user defined command line argument.
 */


/* GSL RNG types */

#define GGEN_RNG_MT19937	0
#define GGEN_RNG_RANMAR		1
#define GGEN_RNG_MAX		2

#define GGEN_RNG_TESTING GGEN_RNG_MAX+1
#define GGEN_RNG_DEFAULT GGEN_RNG_RANMAR

/******************************************************************************
 * GGEN GSL Random Number Generator,
 * Handle all creation and access to gsl_rng
******************************************************************************/

/** A Random Number Generator
 * This is in fact a big wrapper to the GSL library. Go look at the GNU Scientific Library
 * documentation for any additional information.
 */
class ggen_rng {
	public:
		/** Creates an empty ggen_rng
		 */
		ggen_rng();
		
		/** Destroys the rng
		 * Also destroy the GSL rng and save its state if possible
		 */
		~ggen_rng();
	
		/** Allocates the gsl backend in function of the type asked
		 * @param rng_type : a GGEN_RNG_TYPE between 0 and GGEN_RNG_MAX
		 */
		int allocate(const unsigned int rng_type);

		/** Sets the rng seed
		 * @param s : the seed to use
		 */
		void seed(unsigned long int s);

		/** Retreives the backend rng
		 * We actually use GSL for random number generation and this 
		 * allows the inspection of the backend
		 * @return the GSL random number generator
		 */
		const gsl_rng* get_gsl();
	
		/** Sets the file to be used for loading/saving the generator state.
		 * @param filename : the name of the file
		 */
		void set_file(char *filename);

		/** Reads the generator state from a given file
		 * This fail with a warning if the file connot be openned.
		 */
		void read();
		
		/** Writes the generator state to a specific file
		 * Fails with a warning if the rng cannot be saved
		 */
		void write();
		
		/** Chooses k elements in an array of size n
		 * @param dest : the destination array
		 * @param k : the size of the destination array
		 * @param src : the source array
		 * @param n : the size of the source array
		 * @param size : the size of each element in bytes
		 */
		void choose(boost::any *dest, size_t k, boost::any* src, size_t n,size_t size);
		
		/** Shuffles an array
		 * @param base : the array to shuffle
		 * @param n : the size of the array
		 * @param size : the size of each element of the array
		 */
		void shuffle(boost::any *base, size_t n, size_t size);
		
		/** Runs a bernouilli trial with probability of p
		 * @param p : the probability to obtain true
		 * @return true with probability p and false with probability 1 - p
		 */
		bool bernoulli(double p = 0.5);
		
		/** Chooses uniformly an integer in the range 0 n-1.
		 * @param n : the upper limit of the range
		 * @return an integer between 0 and n-1, choosed uniformly
		 */
		unsigned long int uniform_int(unsigned long int n);
	protected:
		/** The GSL backend random number generator*/
		gsl_rng* rng;

		/** the file used to save the state */
		char *file;
};

/** Class used for testing purposes only.
 * Implements all rng functions in a deterministic way
 */
class ggen_rng_testing : public ggen_rng {
		public:
		/** Creates a ggen_rng_testing
		 */
		ggen_rng_testing();
		
		/** Destroys the rng
		 * Also destroy the GSL rng
		 */
		~ggen_rng_testing();
		
		/** Should return the backend rng
		 * As no backend exists, return always NULL
		 * @return NULL
		 */
		const gsl_rng* get_gsl();
		
		/** Should read the generator state from a given file
		 * This always fail.
		 * @param filename : the file containing the rng. This file is in binary GSL RNG format.
		 */
		void read(std::string filename);
		
		/** Should write the generator state to a specific file
		 * Always fail.
		 * @param filename : the file which will contain the saved state
		 */
		void write(std::string filename);
		
		/** Chooses THE FIRST k elements in an array of size n
		 * @param dest : the destination array
		 * @param k : the size of the destination array
		 * @param src : the source array
		 * @param n : the size of the source array
		 * @param size : the size of each element in bytes
		 */
		void choose(boost::any *dest, size_t k, boost::any* src, size_t n,size_t size);
		
		/** Should shuffle an array. To actually NOTHING.
		 * @param base : the array to shuffle
		 * @param n : the size of the array
		 * @param size : the size of each element of the array
		 */
		void shuffle(boost::any *base, size_t n, size_t size);
		
		/** Should run a bernouilli trial with probability of p.
		 * Implemented by an alterning sequence.
		 * @param p : the probability to obtain true
		 * @return true, false, true, false, true, ...
		 */
		bool bernoulli(double p = 0.5);
		
		/** Should choose uniformly an integer in the range 0 n-1.
		 * Implemented by a deterministic algorithm.
		 * @param n : the upper limit of the range
		 * @return TODO
		 */
		unsigned long int uniform_int(unsigned long int n);
	private:
		/** used for the alterning sequence returned by bernoulli()*/
		bool b;
};

/* *****************************************************************************
 * GGEN GSL Random Number Distribution,
 * Handle all creation and access to gsl_rnd
******************************************************************************/

/** This ggen_rnd class is responsible of the creation of all supported random number distribution
 * It contains a protected random number generator common to all derived classes as all gsl_rnd
 * need a gsl_rng to work.
 */
class ggen_rnd {
	public:
		/** A base contructor, doesn't do very much
		 * @param r : the rng behind this rnd
		 */
		ggen_rnd(ggen_rng* r);
		
		/** Each distribution should implement a get() function returning the
		 * next random number fitting the distribution
		 * @return a random number so that all random numbers returned fit a specific distribution
		 */
		virtual double get() = 0;
		
		/** Base destructor
		 */
		~ggen_rnd();		
		
	protected:
		/** The random number generator used for the generation of a given distribution*/
		ggen_rng* rng;
};

/* Gaussian distribution
 ************************/

/** Implements the gaussian distribution
 * Actually a wrapper for gsl gaussian
 */
class ggen_rnd_gaussian : public ggen_rnd {
	public:
		/** Constructor
		 * @param rng: the rng associated with this rnd
		 * @param s : the center of the distribution
		 */
		ggen_rnd_gaussian(ggen_rng* rng, double s);
	
		/** Gets the next random number fitting the distribution
		 * @return a random number
		 */
		double get();
	private:
		/** The gaussian distribution will be centered on this value*/
		double sigma;
};

/* Flat (unifrom) distribution
 ******************************/

/** Implements the uniform distribution
 */
class ggen_rnd_flat : public ggen_rnd {
	public:
		
		/** Constructor
		 * @param rng: the rng associated with this rnd
		 * @param mi : the minimum of the distribution
		 * @param mx : the maximum of the distribution
		 */
		ggen_rnd_flat(ggen_rng* rng, double mi, double mx);

		/** Returns the next double in the distribution
		 * @return a double between min and max chosen uniformly
		 */
		double get();
	private:
		/** The bottom limit of this uniform distribution*/
		double min;
		
		/** The upper limit of this uniform distribution*/
		double max;
};

/* Exponential distribution
 ******************************/

/** Implements the exponential distribution
 */
class ggen_rnd_exponential : public ggen_rnd {
	public:
		
		/** Constructor
		 * @param rng: the rng associated with this rnd
		 * @param mu : the mean of the distribution
		 */
		ggen_rnd_exponential(ggen_rng* rng, double mu);

		/** Returns the next double in the distribution
		 * @return a double between min and max chosen uniformly
		 */
		double get();
	private:
		/** The mean of this distribution*/
		double mu;
};

};

#endif
