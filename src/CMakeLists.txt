# ggen project
PROJECT(ggen CXX C)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR})

# For boost
SET(Boost_USE_STATIC_LIBS OFF)
SET(Boost_USE_MULTITHREAD OFF)
FIND_PACKAGE(Boost REQUIRED regex program_options graph)

# For GSL
find_package(GSL)

ADD_EXECUTABLE(graph_generator graph_generator.cpp dynamic_properties.cpp random.cpp)
ADD_EXECUTABLE(property_adder property_adder.cpp random.cpp  dynamic_properties.cpp)
TARGET_LINK_LIBRARIES(graph_generator ${Boost_LIBRARIES} ${GSL_LIBRARIES})
target_link_libraries(property_adder ${Boost_LIBRARIES} ${GSL_LIBRARIES})
ADD_EXECUTABLE(analysis analysis.cpp dynamic_properties.cpp)
TARGET_LINK_LIBRARIES(analysis ${Boost_LIBRARIES} ${GSL_LIBRARIES})

