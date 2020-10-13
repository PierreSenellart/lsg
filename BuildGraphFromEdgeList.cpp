/* $Id: BuildGraphFromEdgeList.cpp 8147 2013-07-09 08:29:13Z pierre $ */

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

#include "MutableGraph.h"

using namespace std;
using namespace lsg;

int main(int argc, char **argv)
{
  if(argc!=4) {
    cerr << "Usage : " << argv[0] << " edge_list labels graph" << endl;
    return EXIT_FAILURE;
  }

  ifstream edge_list(argv[1]);

  cerr << "Loading edge list..." << endl;
  MutableGraph g(edge_list);

  cerr << "Adding labels..." << endl;
  ifstream labels(argv[2]);
  node_t size=g.getNbNodes();
  for(node_t i=0;i<size && labels;++i) {
    string s;
    getline(labels,s);
    while(s.size() && (s[s.size()-1] == '\r' || s[s.size()-1] == '\n')) {
      s = s.substr( 0, s.size() - 1 );
    }
    g.setLabel(i,s);
  }

  cerr << "Storing graph..." << endl;
  g.store(argv[3]);

  return EXIT_SUCCESS;
}
