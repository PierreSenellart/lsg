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

#include "PackedGraph.h"
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <unordered_set>
#include <stack>

using namespace lsg;

std::unordered_set<node_t> getAncestors(const Graph &g, node_t node)
{
  std::unordered_set<node_t> ancestors;
  std::unordered_set<node_t> toVisit;
  toVisit.insert(node);

  int count {0};

  while(!toVisit.empty())
  {
    auto n = *toVisit.begin();
    toVisit.erase(toVisit.begin());

    if(++count % 100 == 0)
      std::cerr << "." << std::flush;

    ancestors.insert(n);

    for(auto it=g.column(n).begin(), itend=g.column(n).end();
        it!=itend;
        ++it) {
      if(ancestors.find(it.index())==ancestors.end() &&
         toVisit.find(it.index())==toVisit.end())
        toVisit.insert(it.index());
    }
  }

  std::cerr << "\n";

  ancestors.erase(node);
  return ancestors;
}

int main(int argc, char **argv) {
  if(argc<3) {
    std::cerr << "Usage: " << argv[0] << " graph node_label ..." << std::endl;
    return EXIT_FAILURE;
  }

  PackedGraph g(argv[1]);

  for(auto i = 2; i<argc; ++i) {
    auto node = g.getNodeWithLabel(argv[i]);

    std::cout << argv[i] << "\t";

    if(node == static_cast<node_t>(-1))
      std::cout << "Not found" << std::endl;
    else
      std::cout << g.inDegree(node) << "\t" << getAncestors(g, node).size() << std::endl;
  }

  return EXIT_SUCCESS;
}
