#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/random.hpp>
#include <boost/random/linear_congruential.hpp>

using namespace boost;

typedef adjacency_list<vecS, vecS, bidirectionalS,
		       no_property, no_property > Graph;

Graph *generate_graph(int num_vertices, int num_edges) {

  Graph *g = new Graph();
  rand48 r = rand48((uint64_t) 0);
  generate_random_graph(*g,num_vertices,num_edges,r,false,false);
  return g;
}


Graph *gen2() {
  Graph *g = new Graph(2);
  int t[2][2] = {{1,0}, {0,0}};

  for (int i=0; i<2; i++)
    for (int j=0; j<2; j++)
      if(t[i][j] == 1)
	add_edge(i,j,*g);

  return g;
}


int main(int argc, char** argv)
{
  if(argc != 3) 
    return 0;

  

  int num_vertices = atoi(argv[1]);
  int num_edges = atoi(argv[2]);

  //Graph *g = generate_graph(num_vertices, num_edges);
  Graph *g = gen2();

  write_graphviz(std::cout, *g);

  return 0;
}
