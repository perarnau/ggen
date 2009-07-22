
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

/* This program tests a number of expected properties
 * of the generation algorithms inplemented
 */

#include <boost/config.hpp>


#include "../ggen.hpp"
#include "../random.hpp"
#include "../graph_generator.hpp"


int main(int argc, char **argv)
{
	// Handle command line arguments
	////////////////////////////////////
	po::options_description od_general("General Options");
	od_general.add_options()
		("test", po::value<std::string>(), "test to execute")
		("dag", po::value<bool>()->zero_tokens(),"test the dag version")
	;

	// Positional Options
	///////////////////////////////

	po::positional_options_description pod_methods;
	pod_methods.add("test", 1);

	// Options parsing
	///////////////////////////////

	po::variables_map vm_general;
	po::parsed_options prso_general = po::command_line_parser(argc,argv).options(od_general).positional(pod_methods).run();
	po::store(prso_general,vm_general);
	po::notify(vm_general);
	
	// Initialisaton, if this fail nothing passes
	/////////////////////////////////////////////
	
	Graph *g = NULL;
	ggen_rng *r = NULL;
	Graph_generation_context *cntxt = new Graph_generation_context();
	
	// Launching tests
	//////////////////////////////
	
	if (vm_general.count("test")) {
		std::string to_test = vm_general["test"].as<std::string>();
		if(to_test == "erdos_gnp")
		{
			r = new ggen_rng_testing();
			cntxt->set_rng(r);
			bool do_dag;
			if(vm_general.count("dag"))
				do_dag = true;
			else
				do_dag =false;

			g = Graph_generation::gg_erdos_gnp(*cntxt,10,0.5,do_dag);
			return g != NULL;
		}
		else
			std::cerr << "Wrong test name" << std::endl;
	}
	else {
		std::cerr << "No test name" << std::endl;
		return -1;
	}
	return 0;
}
