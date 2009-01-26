/*=============================================================================
  Copyright (c) 2002-2003 Joel de Guzman
http://spirit.sourceforge.net/

Use, modification and distribution is subject to the Boost Software
License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
///////////////////////////////////////////////////////////////////////////////
//
//  This sample demontrates a parser for a comma separated list of numbers
//  This is discussed in the "Quick Start" chapter in the Spirit User's Guide.
//
//  [ JDG 5/10/2002 ]
//
///////////////////////////////////////////////////////////////////////////////
#define BOOST_SPIRIT_DEBUG
#include <boost/spirit.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/any.hpp>
#include <boost/spirit/actor/insert_key_actor.hpp>
#include <iostream>
#include <vector>
#include <string>


///////////////////////////////////////////////////////////////////////////////
using namespace std;
using namespace boost::spirit;

////////////////////////////////////////////////////////////////////////////
////
////  Types
////
//////////////////////////////////////////////////////////////////////////////
typedef char                    char_t;
typedef file_iterator<char_t>   iterator_t;
typedef scanner<iterator_t>     scanner_t;
typedef rule<scanner_t>         rule_t;

//////////////////////////////////////////////////////////////////////////////
////
////  Actions
////
//////////////////////////////////////////////////////////////////////////////
void echo(iterator_t first, iterator_t const& last)
{
	while (first != last)
	std::cout << *first++;
}


///////////////////////////////////////////////////////////////////////////////
//
//  Our config parser
//
///////////////////////////////////////////////////////////////////////////////

struct config_parser : public grammar<config_parser>
{
	int min,max;
	std::map<std::string,int > vertex_map,edge_map;
	std::map<std::string,int >::value_type gen;
	
	config_parser() {
	}

	template <typename scanner_t> struct definition
	{
		definition(config_parser const& self)
		{
			attribute_distribution = chseq_p("uni_dist") >> 
						'(' >> 
							int_p[assign_a(self.min)] >> 
							',' >> int_p[assign_a(self.max)] >> 
						')'
						;
				 
			attribute_name = (+alnum_p)[assign_a(self.gen.first)];

			attribute_definition = attribute_name >> ':' >> (attribute_distribution)[ assign_a( self.gen.second ,self.min ) ];
			
			vertex = chseq_p("vertex") >> '{' >> *(attribute_definition[insert_key_a(self.vertex_map,self.gen)]) >> '}';
			
			edge = chseq_p("edge") >> '{' >> *(attribute_definition[insert_key_a(self.edge_map,self.gen)]) >> '}'; 

			config_file = vertex >> edge >> !end_p;

			BOOST_SPIRIT_DEBUG_RULE(attribute_distribution);
			BOOST_SPIRIT_DEBUG_RULE(attribute_definition);
			BOOST_SPIRIT_DEBUG_RULE(vertex);
			BOOST_SPIRIT_DEBUG_RULE(edge);
			BOOST_SPIRIT_DEBUG_RULE(config_file);
		}

		rule<scanner_t> attribute_distribution,attribute_name, attribute_definition, vertex, edge , config_file;
		rule<scanner_t> const& start() { return config_file; }

	};
};

////////////////////////////////////////////////////////////////////////////
//
//  Main program
//
////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
	if (2 > argc)
	{
		std::cout << "Must specify a filename!\n";
		return -1;
	}


	cout << "/////////////////////////////////////////////////////////\n\n";
	cout << "\t\tA config parser for graph generation using SPIRIT...\n\n";
	cout << "/////////////////////////////////////////////////////////\n\n";

	config_parser config_grammar;
	BOOST_SPIRIT_DEBUG_NODE(config_grammar);

	// Create a file iterator for this file
	iterator_t first(argv[1]);

	if (!first)
	{
		std::cout << "Unable to open file!\n";
		return -1;
	}

	// Create an EOF iterator
	iterator_t last = first.make_end();

	// Parse
	parse_info <iterator_t> info = parse(first,last,config_grammar,space_p);

	// This really shouldn't fail...
	if (info.full)
		std::cout << "Parse succeeded!\n";
	else
		std::cout << "Parse failed! : " << info.stop << "\n";
	
	return 0;
}



