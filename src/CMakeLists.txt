# ggen project
PROJECT(ggen CXX C)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# Pour boost
SET(Boost_USE_STATIC_LIBS OFF)
SET(Boost_USE_MULTITHREAD OFF)
FIND_PACKAGE(Boost REQUIRED regex program_options graph)

ADD_EXECUTABLE(ggen ggen.cpp dynamic_properties.cpp)
TARGET_LINK_LIBRARIES(ggen ${Boost_LIBRARIES})

