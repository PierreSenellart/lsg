/*
 *  Copyright (c) 2006 Yann Ollivier <yann.ollivier@normalesup.org>
 *                     Pierre Senellart <pierre@senellart.com>
 *  
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to permit
 *  persons to whom the Software is furnished to do so, subject to the
 *  following conditions:
 *  
 *  The above copyright notice and this permission notice shall be included
 *  in all copies or substantial portions of the Software.
 *  
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 *  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 *  NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 *  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 *  OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 *  USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <algorithm>

#include "MutableGraph.h"
#include "PackedGraph.h"
#include "ConnectedComponents.h"

using namespace std;
using namespace lsg;

int main(int argc, char **argv)
{
  if(argc!=3) {
    cerr << "Usage : " << argv[0] << " graph_in graph_out" << endl;
    return EXIT_FAILURE;
  }

  cerr << "Loading graph..." << endl;

  PackedGraph g(argv[1]);

  if(!g.isOk()) {
    cerr << "Impossible to load graph..." << endl;
    return EXIT_FAILURE;
  }
  
  cerr << "Computing strongly connected components..." << endl;

  std::vector<bool> largest_scc(g.getNbNodes());

  {
    /* Let's put this in a block so that all used memory is freed
     * afterwards */

    std::vector<node_t> comp;
    stronglyConnectedComponents(g,comp);

    node_t nb_components=*max_element(comp.begin(),comp.end());
    cerr << "  " << nb_components
                << " different components" << endl;

    cerr << "Computing largest component..." << endl;
    std::vector<node_t> frequency(nb_components);
    const node_t size=g.getNbNodes();
    for(node_t i=0;i<size;++i) 
      ++frequency[comp[i]-1];

    node_t largest_component=max_element(frequency.begin(),frequency.end())
                            - frequency.begin();

    transform(comp.begin(),comp.end(),
              largest_scc.begin(),
              bind2nd(equal_to<node_t>(),largest_component+1));
  }

  cerr << "Storing largest component subgraph..." << endl;
  g.storeSubgraph(argv[2],largest_scc);

  return EXIT_SUCCESS;
}
