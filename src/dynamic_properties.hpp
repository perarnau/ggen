#ifndef DYNAMIC_PROPERTIES_H
#define DYNAMIC_PROPERTIES_H

#include <boost/config.hpp>
#include <boost/dynamic_property_map.hpp>

#include "ggen.hpp"

void create_default_vertex_property(boost::dynamic_properties& dp, Graph& g,const char *name = "node_id");


#endif
